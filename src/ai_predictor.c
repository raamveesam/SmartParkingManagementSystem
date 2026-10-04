#include "parking_system.h"

/*
 * Baseline Diurnal Traffic Curve
 * Represents standard urban parking utilization curve across 24 hours (00:00 to 23:00).
 * Values indicate expected occupancy fractions (0.00 to 1.00).
 */
static const double URBAN_BASELINE_DEMAND[24] = {
    0.12, 0.08, 0.05, 0.05, 0.08, 0.15,  /* 00:00 - 05:00: Overnight minimal */
    0.35, 0.65, 0.88, 0.92, 0.84, 0.78,  /* 06:00 - 11:00: Morning commute peak */
    0.72, 0.75, 0.70, 0.68, 0.82, 0.95,  /* 12:00 - 17:00: Afternoon & evening peak */
    0.89, 0.76, 0.58, 0.42, 0.28, 0.18   /* 18:00 - 23:00: Night tapering */
};

/*
 * Trains the AI demand model using Ordinary Least Squares (OLS) regression
 * combined with historical session timestamps recorded in the system.
 */
void train_ai_demand_model(ParkingSystem* sys) {
    if (!sys) return;

    AIDemandModel* model = &(sys->aiModel);

    /* Populate or blend with historical transaction logs */
    double totalArrivals = 0.0;
    for (int h = 0; h < 24; h++) {
        totalArrivals += model->hourlyHistoricalCount[h];
    }

    /* Compute empirical occupancy fraction per hour */
    for (int h = 0; h < 24; h++) {
        if (totalArrivals > 5.0) {
            /* Blend recorded traffic with baseline curve using 60/40 weighted heuristic */
            double empiricalWeight = model->hourlyHistoricalCount[h] / (totalArrivals / 24.0);
            double normalizedObserved = empiricalWeight * 0.50;
            if (normalizedObserved > 1.0) normalizedObserved = 1.0;

            model->hourlyAverageOccupancy[h] = (0.60 * URBAN_BASELINE_DEMAND[h]) +
                                               (0.40 * normalizedObserved);
        } else {
            /* Fallback to baseline prior distribution when dataset is small */
            model->hourlyAverageOccupancy[h] = URBAN_BASELINE_DEMAND[h];
        }
    }

    /* Fit Linear Regression trend across active business day hours (06:00 - 20:00) */
    double sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;
    int nPoints = 0;

    for (int h = 6; h <= 20; h++) {
        double x = (double)h;
        double y = model->hourlyAverageOccupancy[h];
        sumX += x;
        sumY += y;
        sumXY += (x * y);
        sumXX += (x * x);
        nPoints++;
    }

    double denominator = (nPoints * sumXX - sumX * sumX);
    if (fabs(denominator) > 1e-6) {
        model->regressionSlope = (nPoints * sumXY - sumX * sumY) / denominator;
        model->regressionIntercept = (sumY - model->regressionSlope * sumX) / nPoints;
    } else {
        model->regressionSlope = 0.0;
        model->regressionIntercept = 0.5;
    }

    model->isTrained = true;
    printf("[AI Engine] Demand model successfully trained with %d historical transactions.\n",
           sys->totalTransactions);
}

/*
 * Evaluates predicted demand and dynamic pricing surge factor for a target hour.
 */
void predict_demand_for_hour(const ParkingSystem* sys, int hour, double* outDemandPct, double* outSurgeMultiplier) {
    if (hour < 0 || hour > 23) {
        hour = 12;
    }

    double demand = URBAN_BASELINE_DEMAND[hour];

    if (sys && sys->aiModel.isTrained) {
        demand = sys->aiModel.hourlyAverageOccupancy[hour];
    }

    /* Clamp demand between 5% and 99% */
    if (demand < 0.05) demand = 0.05;
    if (demand > 0.99) demand = 0.99;

    if (outDemandPct) {
        *outDemandPct = demand * 100.0;
    }

    /* Dynamic Pricing / Surge Multiplier heuristic based on anticipated load */
    double surge = 1.0;
    if (demand >= 0.85) {
        surge = 1.50; /* Peak Congestion: 50% surge to encourage turnaround */
    } else if (demand >= 0.70) {
        surge = 1.25; /* Moderate Rush: 25% surge */
    } else if (demand <= 0.25) {
        surge = 0.85; /* Off-Peak Discount: 15% discount to attract vehicles */
    } else {
        surge = 1.00; /* Regular tariff */
    }

    if (outSurgeMultiplier) {
        *outSurgeMultiplier = surge;
    }
}

/*
 * Visualizes a 24-hour demand prediction chart with ASCII bar graph and tariff adjustments.
 */
void display_ai_demand_dashboard(const ParkingSystem* sys) {
    printf("\n");
    printf("=========================================================================================\n");
    printf("                  AI PARKING DEMAND PREDICTION & SURGE DASHBOARD                         \n");
    printf("=========================================================================================\n");
    printf("Hour  | Forecast Demand | Load Bar Graph                          | Pricing Tier\n");
    printf("------+-----------------+-----------------------------------------+----------------------\n");

    for (int h = 0; h < 24; h++) {
        double demandPct = 0.0;
        double surgeFactor = 1.0;
        predict_demand_for_hour(sys, h, &demandPct, &surgeFactor);

        int barBlocks = (int)(demandPct / 3.33); /* Max 30 blocks for 100% */
        if (barBlocks < 1) barBlocks = 1;
        if (barBlocks > 30) barBlocks = 30;

        char bar[32];
        for (int b = 0; b < barBlocks; b++) {
            bar[b] = '#';
        }
        bar[barBlocks] = '\0';

        const char* tag;
        if (surgeFactor > 1.25) {
            tag = "HIGH SURGE  (1.50x)";
        } else if (surgeFactor > 1.0) {
            tag = "MODERATE    (1.25x)";
        } else if (surgeFactor < 1.0) {
            tag = "OFF-PEAK    (0.85x)";
        } else {
            tag = "STANDARD    (1.00x)";
        }

        printf("%02d:00 | %6.1f%%        | %-30s  | %s\n", h, demandPct, bar, tag);
    }

    printf("------+-----------------+-----------------------------------------+----------------------\n");
    printf("Insight: Peak congestion expected around 08:00 - 10:00 and 17:00 - 19:00.\n");
    printf("Smart Recommendation: Reserve Floor 2 overflow capacity during peak windows.\n\n");
}
