"""
BayTrack Project Documentation Generator
Produces: Technical Report + Diagrams + Algorithms + Complexity Analysis
"""

import os, subprocess
from docx import Document
from docx.shared import Inches, Pt, Cm, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml import parse_xml
from docx.oxml.ns import nsdecls, qn
from docx.oxml import OxmlElement

# ── Paths ──────────────────────────────────────────────────────────────────────
REPO    = r"C:\Users\RamLaxma\.gemini\antigravity\scratch\SmartParkingManagementSystem"
BRAIN   = r"C:\Users\RamLaxma\.gemini\antigravity\brain\85bebf8f-b802-4dd7-a029-a21834061d11"
IMG_SYS = os.path.join(BRAIN, "system_architecture_1792929841558.png")
IMG_FLW = os.path.join(BRAIN, "checkin_flowchart_1792929871619.png")
IMG_DS  = os.path.join(BRAIN, "data_structures_diagram_1792929917280.png")
OUT     = os.path.join(REPO,  "BayTrack_Project_Report.docx")

# ── Colour palette ─────────────────────────────────────────────────────────────
C_NAVY  = RGBColor(30, 58, 138)
C_BLUE  = RGBColor(37, 99, 235)
C_SLATE = RGBColor(71, 85, 105)
C_DARK  = RGBColor(15, 23, 42)
C_WHITE = RGBColor(255, 255, 255)

# ── Helpers ────────────────────────────────────────────────────────────────────

def shade_cell(cell, hex_color: str):
    tcPr = cell._tc.get_or_add_tcPr()
    tcPr.append(parse_xml(f'<w:shd {nsdecls("w")} w:fill="{hex_color}"/>'))

def border_cell(cell, top="D1D5DB", left="1D4ED8", right="D1D5DB", bottom="D1D5DB",
                left_sz=18, other_sz=4):
    tcPr = cell._tc.get_or_add_tcPr()
    tcPr.append(parse_xml(
        f'<w:tcBorders {nsdecls("w")}>'
        f'<w:top    w:val="single" w:sz="{other_sz}" w:color="{top}"/>'
        f'<w:left   w:val="single" w:sz="{left_sz}"  w:color="{left}"/>'
        f'<w:bottom w:val="single" w:sz="{other_sz}" w:color="{bottom}"/>'
        f'<w:right  w:val="single" w:sz="{other_sz}" w:color="{right}"/>'
        f'</w:tcBorders>'))

def pad_cell(cell, t=80, b=80, l=120, r=120):
    tcPr = cell._tc.get_or_add_tcPr()
    tcPr.append(parse_xml(
        f'<w:tcMar {nsdecls("w")}>'
        f'<w:top w:w="{t}" w:type="dxa"/><w:bottom w:w="{b}" w:type="dxa"/>'
        f'<w:left w:w="{l}" w:type="dxa"/><w:right w:w="{r}" w:type="dxa"/>'
        f'</w:tcMar>'))

def set_table_border(table, color="CBD5E1"):
    """Add a thin border around the entire table."""
    tbl = table._tbl
    tblPr = tbl.find(qn('w:tblPr'))
    if tblPr is None:
        tblPr = OxmlElement('w:tblPr')
        tbl.insert(0, tblPr)
    tblBorders = parse_xml(
        f'<w:tblBorders {nsdecls("w")}>'
        f'<w:top    w:val="single" w:sz="4" w:color="{color}"/>'
        f'<w:left   w:val="single" w:sz="4" w:color="{color}"/>'
        f'<w:bottom w:val="single" w:sz="4" w:color="{color}"/>'
        f'<w:right  w:val="single" w:sz="4" w:color="{color}"/>'
        f'<w:insideH w:val="single" w:sz="4" w:color="{color}"/>'
        f'<w:insideV w:val="single" w:sz="4" w:color="{color}"/>'
        f'</w:tblBorders>')
    tblPr.append(tblBorders)

def add_code(doc, text: str, font_size=8.5):
    tbl  = doc.add_table(rows=1, cols=1)
    cell = tbl.cell(0, 0)
    shade_cell(cell, "F1F5F9")
    border_cell(cell)
    pad_cell(cell, 90, 90, 150, 150)
    p    = cell.paragraphs[0]
    p.paragraph_format.line_spacing = 1.15
    run  = p.add_run(text)
    run.font.name  = "Consolas"
    run.font.size  = Pt(font_size)
    run.font.color.rgb = RGBColor(30, 41, 59)
    doc.add_paragraph().paragraph_format.space_after = Pt(6)

def heading1(doc, text: str):
    p = doc.add_paragraph()
    p.paragraph_format.space_before = Pt(18)
    p.paragraph_format.space_after  = Pt(6)
    run = p.add_run(text)
    run.font.name  = "Arial"
    run.font.size  = Pt(15)
    run.font.bold  = True
    run.font.color.rgb = C_NAVY
    # Underline rule
    pPr = p._p.get_or_add_pPr()
    pBdr = OxmlElement('w:pBdr')
    bot  = OxmlElement('w:bottom')
    bot.set(qn('w:val'),   'single')
    bot.set(qn('w:sz'),    '4')
    bot.set(qn('w:color'), '2563EB')
    pBdr.append(bot)
    pPr.append(pBdr)

def heading2(doc, text: str):
    p = doc.add_paragraph()
    p.paragraph_format.space_before = Pt(12)
    p.paragraph_format.space_after  = Pt(4)
    run = p.add_run(text)
    run.font.name = "Arial"
    run.font.size = Pt(12)
    run.font.bold = True
    run.font.color.rgb = C_BLUE

def body(doc, text: str, size=10.5, space_after=8):
    p   = doc.add_paragraph(text)
    p.paragraph_format.space_after  = Pt(space_after)
    p.paragraph_format.line_spacing = 1.3
    for run in p.runs:
        run.font.name = "Calibri"
        run.font.size = Pt(size)
        run.font.color.rgb = C_DARK

def bullet(doc, items: list):
    for item in items:
        p = doc.add_paragraph(style='List Bullet')
        p.paragraph_format.space_after = Pt(3)
        run = p.add_run(item)
        run.font.name = "Calibri"
        run.font.size = Pt(10.5)
        run.font.color.rgb = C_DARK

def info_table(doc, headers: list, rows: list, col_widths: list = None):
    tbl = doc.add_table(rows=1+len(rows), cols=len(headers))
    tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
    set_table_border(tbl)

    # Header row
    hdr = tbl.rows[0]
    for i, h in enumerate(headers):
        c = hdr.cells[i]
        shade_cell(c, "1E3A8A")
        pad_cell(c, 80, 80, 100, 100)
        p = c.paragraphs[0]
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        run = p.add_run(h)
        run.font.name = "Arial"
        run.font.size = Pt(9.5)
        run.font.bold = True
        run.font.color.rgb = C_WHITE

    # Data rows
    for ri, row_data in enumerate(rows):
        fill = "F8FAFC" if ri % 2 == 0 else "EFF6FF"
        for ci, cell_text in enumerate(row_data):
            c = tbl.rows[ri+1].cells[ci]
            shade_cell(c, fill)
            pad_cell(c, 70, 70, 100, 100)
            p = c.paragraphs[0]
            p.alignment = WD_ALIGN_PARAGRAPH.CENTER
            run = p.add_run(str(cell_text))
            run.font.name = "Calibri"
            run.font.size = Pt(9.5)
            run.font.color.rgb = C_DARK

    if col_widths:
        for i, w in enumerate(col_widths):
            for row in tbl.rows:
                row.cells[i].width = Inches(w)

    doc.add_paragraph().paragraph_format.space_after = Pt(8)
    return tbl

def embed_image(doc, path: str, caption: str, width=5.8):
    if os.path.exists(path):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        run = p.add_run()
        run.add_picture(path, width=Inches(width))
        cap = doc.add_paragraph(f"Figure: {caption}")
        cap.alignment = WD_ALIGN_PARAGRAPH.CENTER
        cap.paragraph_format.space_after = Pt(12)
        for run in cap.runs:
            run.font.name  = "Calibri"
            run.font.size  = Pt(9)
            run.font.italic = True
            run.font.color.rgb = C_SLATE
    else:
        body(doc, f"[Image not found: {path}]")

# ══════════════════════════════════════════════════════════════════════════════
#  DOCUMENT CONSTRUCTION
# ══════════════════════════════════════════════════════════════════════════════

def build_document():
    doc = Document()

    # Page layout – A4 with comfortable margins
    for sec in doc.sections:
        sec.page_height = Cm(29.7)
        sec.page_width  = Cm(21.0)
        sec.top_margin  = sec.bottom_margin = Inches(1.0)
        sec.left_margin = sec.right_margin  = Inches(1.1)

    # ── COVER PAGE ────────────────────────────────────────────────────────────
    cover = doc.add_paragraph()
    cover.alignment = WD_ALIGN_PARAGRAPH.CENTER
    cover.paragraph_format.space_before = Pt(60)
    r = cover.add_run("BAYTRACK")
    r.font.name = "Arial"; r.font.size = Pt(36)
    r.font.bold = True;    r.font.color.rgb = C_NAVY

    sub = doc.add_paragraph("Smart Parking Management System")
    sub.alignment = WD_ALIGN_PARAGRAPH.CENTER
    sub.paragraph_format.space_after = Pt(4)
    for run in sub.runs:
        run.font.name = "Arial"; run.font.size = Pt(18)
        run.font.color.rgb = C_BLUE

    tagline = doc.add_paragraph("Project Report  |  Diagrams  |  Algorithms  |  Complexity Analysis")
    tagline.alignment = WD_ALIGN_PARAGRAPH.CENTER
    tagline.paragraph_format.space_after = Pt(32)
    for run in tagline.runs:
        run.font.name = "Calibri"; run.font.size = Pt(12)
        run.font.italic = True;    run.font.color.rgb = C_SLATE

    meta_lines = [
        ("Language",       "C  (C99 Standard)"),
        ("Repository",     "github.com/raamveesam/SmartParkingManagementSystem"),
        ("Author",         "raamveesam"),
        ("Subject",        "Data Structures & Algorithms Design Project"),
        ("Date",           "October 2026"),
    ]
    mtbl = doc.add_table(rows=len(meta_lines), cols=2)
    mtbl.alignment = WD_TABLE_ALIGNMENT.CENTER
    set_table_border(mtbl, "CBD5E1")
    for ri, (lbl, val) in enumerate(meta_lines):
        fill = "EFF6FF" if ri % 2 == 0 else "F8FAFC"
        c0, c1 = mtbl.rows[ri].cells[0], mtbl.rows[ri].cells[1]
        shade_cell(c0, "1E3A8A"); shade_cell(c1, fill)
        pad_cell(c0, 80, 80, 120, 120); pad_cell(c1, 80, 80, 120, 120)
        r0 = c0.paragraphs[0].add_run(lbl)
        r0.font.name="Arial"; r0.font.size=Pt(10); r0.font.bold=True
        r0.font.color.rgb = C_WHITE
        r1 = c1.paragraphs[0].add_run(val)
        r1.font.name="Calibri"; r1.font.size=Pt(10)
        r1.font.color.rgb = C_DARK
        c0.width = Inches(1.8); c1.width = Inches(4.4)

    doc.add_page_break()

    # ── SECTION 1: ABSTRACT ───────────────────────────────────────────────────
    heading1(doc, "1.  Abstract")
    body(doc,
        "BayTrack is a multi-level parking management system implemented entirely in "
        "standard C (C99). The system manages a 20-bay, 2-floor parking lot and "
        "demonstrates the practical application of fundamental data structures — "
        "a static array for bay storage, a singly linked list for billing records, "
        "and a FIFO queue for overflow vehicle management — combined with classical "
        "searching and sorting algorithms. An AI demand heuristic based on a sum of "
        "Gaussian bell curves predicts hourly congestion and computes dynamic pricing "
        "multipliers, simulating real-world smart parking behaviour.\n\n"
        "The system handles vehicle check-in, slot allocation, overflow queuing, "
        "checkout billing with dynamic tariffs, session reporting, and 24-hour "
        "demand forecasting through an interactive command-line interface.")

    # ── SECTION 2: INTRODUCTION ───────────────────────────────────────────────
    heading1(doc, "2.  Introduction")
    body(doc,
        "Rapid urbanisation has created persistent parking congestion in Indian cities. "
        "Manual management of parking lots leads to inefficient space utilisation, "
        "long vehicle queues, and revenue leakage. A software-driven parking management "
        "system can automate slot tracking, reduce waiting times, and apply dynamic "
        "pricing based on real-time demand signals.")
    body(doc,
        "BayTrack addresses this problem through a carefully chosen set of data "
        "structures and algorithms that balance simplicity with correctness:")
    bullet(doc, [
        "Arrays offer O(1) index access for bay status checks.",
        "A linked list accumulates billing records without pre-allocating fixed memory.",
        "A FIFO queue guarantees fairness (first-come-first-served) when the lot is full.",
        "Insertion sort and binary search demonstrate classical DSA applied to real data.",
        "A Gaussian AI heuristic removes the need for an external ML library while "
        "still producing a smooth, realistic demand curve."
    ])

    # ── SECTION 3: PROBLEM STATEMENT ─────────────────────────────────────────
    heading1(doc, "3.  Problem Statement")
    body(doc,
        "Design and implement a multi-floor parking management system in C that satisfies "
        "the following requirements:")
    rows_ps = [
        ["R1", "Vehicle Registration & Bay Allocation",
         "Assign the nearest free bay to an arriving vehicle without duplicates."],
        ["R2", "Overflow Handling",
         "When all category-matching bays are occupied, queue the vehicle fairly."],
        ["R3", "Entry / Exit Tracking",
         "Record precise arrival epoch and compute billable duration on departure."],
        ["R4", "Dynamic Billing",
         "Apply a surge multiplier derived from predicted hourly demand."],
        ["R5", "Slot Search",
         "Locate a parked vehicle's bay using a linear scan of the bay array."],
        ["R6", "Session Reporting",
         "Rank completed sessions by fee using an in-place sort algorithm."],
        ["R7", "AI Demand Prediction",
         "Forecast hourly occupancy using a mathematical model and show a tariff schedule."],
    ]
    info_table(doc, ["ID","Requirement","Description"], rows_ps,
               col_widths=[0.5, 2.0, 3.6])

    # ── SECTION 4: SYSTEM ARCHITECTURE ───────────────────────────────────────
    heading1(doc, "4.  System Architecture")
    body(doc,
        "BayTrack follows a single-file, procedurally organised architecture with four "
        "logical layers. Each layer has a clearly defined responsibility and communicates "
        "only through function calls, keeping coupling low.")
    doc.add_paragraph().paragraph_format.space_after = Pt(4)
    embed_image(doc, IMG_SYS, "BayTrack Four-Layer System Architecture", width=5.5)

    heading2(doc, "4.1  Layer Descriptions")
    rows_layers = [
        ["User Interface", "main() + menu loop",
         "Parses user input and dispatches to module functions."],
        ["Core Modules",   "vehicle_arrive(), vehicle_depart(), find_vehicle()",
         "Business logic: bay allocation, billing, queue management."],
        ["Data Structures","bay_grid[], HoldLine, ExitLog linked list",
         "Underlying storage for all run-time state."],
        ["AI Module",      "peak_score(), pricing_mult()",
         "Gaussian demand model and dynamic tariff computation."],
    ]
    info_table(doc, ["Layer","Components","Responsibility"], rows_layers,
               col_widths=[1.5, 2.2, 2.4])

    # ── SECTION 5: DATA STRUCTURES ────────────────────────────────────────────
    heading1(doc, "5.  Data Structures")
    embed_image(doc, IMG_DS, "BayTrack — Core Data Structures (Array, Queue, Linked List)", width=5.8)

    heading2(doc, "5.1  Static Array  —  bay_grid[BAY_COUNT]")
    body(doc,
        "bay_grid[] is a fixed-size array of ParkBay structs declared at file scope. "
        "Each element represents one physical bay and stores its number, floor level, "
        "vehicle category, occupancy status, parked vehicle plate, pass number, and "
        "arrival timestamp.")
    add_code(doc,
"""typedef struct {
    int   bayNum;        /* bay identifier: 1 .. BAY_COUNT (20)  */
    int   levelNum;      /* 0 = Ground floor, 1 = First floor     */
    int   category;      /* CAT_TW (Two-Wheeler) or CAT_FW        */
    int   status;        /* BAY_FREE = 0  /  BAY_BUSY = 1         */
    char  tag[18];       /* vehicle registration number           */
    int   passNo;        /* entry pass number                     */
    long  arrivalEpoch;  /* unix timestamp of vehicle arrival     */
} ParkBay;

static ParkBay bay_grid[BAY_COUNT];  /* Array: 20 bays, Ground + First floor */""")
    body(doc,
        "Ground floor (level 0): bays 1–6 reserved for Two-Wheelers; bays 7–10 for "
        "Four-Wheelers. First floor (level 1): bays 11–20, all Four-Wheelers.\n"
        "Rationale for array: O(1) index access; fixed capacity known at compile time; "
        "cache-friendly sequential scan for find operations.")

    heading2(doc, "5.2  FIFO Queue  —  HoldLine")
    body(doc,
        "When all bays of a vehicle's category are occupied, the vehicle is appended "
        "to the HoldLine. The queue is implemented as a linked-node structure with "
        "head (front, served next) and tail (rear, most recent addition) pointers. "
        "Enqueue is O(1); dequeue is O(1).")
    add_code(doc,
"""typedef struct HoldNode {
    CarInfo         car;   /* vehicle descriptor                  */
    struct HoldNode *next; /* toward the rear of the queue        */
} HoldNode;

typedef struct {
    HoldNode *head;  /* front — served first (oldest arrival)     */
    HoldNode *tail;  /* rear  — most recently added               */
    int       size;  /* current number of waiting vehicles        */
} HoldLine;""")
    body(doc,
        "When a bay is freed on vehicle departure, the system peeks at the front of "
        "the queue. If the head vehicle's category matches the freed bay, it is "
        "automatically dequeued and assigned — implementing fair first-come-first-served "
        "dispatch with no manual intervention.")

    heading2(doc, "5.3  Singly Linked List  —  ExitLog")
    body(doc,
        "Every completed parking session generates an ExitLog node that is prepended "
        "at the global ledger pointer. Prepending is O(1) and naturally orders the list "
        "from most-recent to oldest — ideal for displaying a billing history.")
    add_code(doc,
"""typedef struct ExitLog {
    int   passNo;        /* entry pass number                     */
    char  tag[18];       /* vehicle registration                  */
    int   category;      /* TW or FW                              */
    int   bayNum;        /* which bay was used                    */
    long  inEpoch;       /* arrival unix timestamp                */
    long  outEpoch;      /* departure unix timestamp              */
    float hoursParked;   /* billable hours (min 1.0)              */
    float peakMult;      /* AI pricing multiplier applied         */
    float feePaid;       /* total amount charged (Rs.)            */
    struct ExitLog *prev;/* chain toward older records            */
} ExitLog;

static ExitLog *ledger = NULL;  /* head of billing linked list    */""")

    # ── SECTION 6: ALGORITHMS ─────────────────────────────────────────────────
    heading1(doc, "6.  Algorithms")

    heading2(doc, "6.1  Bay Assignment — Linear Scan")
    body(doc,
        "assign_bay() traverses bay_grid[] from index 0 to BAY_COUNT–1 and returns "
        "the first bay that is both FREE and matches the requested vehicle category. "
        "This greedy left-to-right scan ensures lower-numbered bays (closer to the "
        "entrance) are always filled first, minimising average walking distance.")
    add_code(doc,
"""Algorithm: ASSIGN_BAY(category, tag, passNo, epoch)
──────────────────────────────────────────────────────
Input  : vehicle category, plate tag, pass number, arrival epoch
Output : assigned bay number, or -1 if lot is full for this category

1.  FOR k = 0 TO BAY_COUNT - 1  DO
2.      IF bay_grid[k].status == BAY_FREE  AND
           bay_grid[k].category == category  THEN
3.          SET bay_grid[k].status        ← BAY_BUSY
4.          SET bay_grid[k].passNo        ← passNo
5.          SET bay_grid[k].arrivalEpoch  ← epoch
6.          COPY tag → bay_grid[k].tag
7.          RETURN bay_grid[k].bayNum
8.      END IF
9.  END FOR
10. RETURN -1    ▷ no matching free bay found""")

    heading2(doc, "6.2  Vehicle Departure & Auto-Queue Dispatch")
    body(doc,
        "vehicle_depart() combines several operations: linear search to locate the "
        "parked vehicle, fee computation using the AI multiplier, ExitLog list prepend, "
        "array cell release, and conditional queue dequeue-and-reassign.")
    add_code(doc,
"""Algorithm: VEHICLE_DEPART(tag, exitEpoch)
──────────────────────────────────────────────────────
1.  foundIdx ← -1
2.  FOR k = 0 TO BAY_COUNT-1  DO          ▷ Linear search
3.      IF bay_grid[k].status == BUSY AND same_tag(bay_grid[k].tag, tag)  THEN
4.          foundIdx ← k ;  BREAK
5.  IF foundIdx == -1  THEN  PRINT warning ; RETURN

6.  rawHrs  ← (exitEpoch - bay_grid[foundIdx].arrivalEpoch) / 3600
7.  billHrs ← MAX(rawHrs, MIN_BILLABLE)
8.  hr      ← hour-of-day(exitEpoch)
9.  sc      ← PEAK_SCORE(hr)              ▷ Gaussian AI model
10. mult    ← PRICING_MULT(sc)
11. fee     ← hourly_rate(category) × billHrs × mult

12. rec ← NEW ExitLog{passNo, tag, category, bayNum, in, out,
                       billHrs, mult, fee}
13. rec.prev ← ledger ;  ledger ← rec     ▷ Prepend to linked list

14. RELEASE bay_grid[foundIdx]            ▷ Clear array cell
15. PRINT receipt

16. IF hold_line.size > 0 AND
       hold_line.head.car.category == freed_category  THEN
17.     waiting ← POP_HOLD()             ▷ Dequeue from front
18.     ASSIGN_BAY(waiting.category, waiting.tag, …)
19.     PRINT auto-allocation notice""")

    heading2(doc, "6.3  Linear Search — Find Vehicle")
    body(doc,
        "find_vehicle() scans bay_grid[] sequentially and returns the bay number of "
        "the first occupied cell whose tag matches the query string (case-insensitive). "
        "Worst case is a full scan of all 20 bays (O(n)).")
    add_code(doc,
"""Algorithm: FIND_VEHICLE(query_tag)
──────────────────────────────────────────────────────
Input  : vehicle registration number string
Output : bay number where vehicle is parked, or -1

1.  FOR k = 0 TO BAY_COUNT - 1  DO
2.      IF bay_grid[k].status == BAY_BUSY  AND
           same_tag(bay_grid[k].tag, query_tag)  THEN
3.          RETURN bay_grid[k].bayNum
4.      END IF
5.  END FOR
6.  RETURN -1""")

    heading2(doc, "6.4  Insertion Sort — Rank Sessions by Fee")
    body(doc,
        "show_sorted_sessions() copies ExitLog pointers into a temporary array, then "
        "applies Insertion Sort in descending order of feePaid. Insertion sort was chosen "
        "because (a) the dataset is small (typical parking lot has tens of sessions), "
        "(b) it is stable — ties are resolved by arrival order, and (c) it is trivially "
        "implementable without a separate comparator function.")
    add_code(doc,
"""Algorithm: INSERTION_SORT_DESC(arr[], n)
──────────────────────────────────────────────────────
Input  : arr[] — array of ExitLog pointers
         n     — number of elements
Output : arr[] sorted descending by feePaid  (in-place)

1.  FOR i = 1 TO n - 1  DO
2.      key ← arr[i]
3.      j   ← i - 1
4.      WHILE j >= 0  AND  arr[j].feePaid < key.feePaid  DO
5.          arr[j+1] ← arr[j]
6.          j ← j - 1
7.      END WHILE
8.      arr[j+1] ← key
9.  END FOR""")

    heading2(doc, "6.5  Binary Search — Pass Number Lookup")
    body(doc,
        "lookup_pass() first sorts the session array by passNo ascending (using "
        "Insertion Sort), then applies classic binary search. Because pass numbers are "
        "monotonically increasing (issued sequentially from TICKET_START = 5001), "
        "the sorted order is guaranteed after one pass.")
    add_code(doc,
"""Algorithm: BINARY_SEARCH_PASS(arr[], n, target)
──────────────────────────────────────────────────────
Input  : arr[]  — ExitLog pointers sorted by passNo ascending
         n      — number of elements
         target — pass number to find
Output : index in arr[] if found, else -1

1.  lo ← 0 ;  hi ← n - 1
2.  WHILE lo <= hi  DO
3.      mid ← lo + (hi - lo) / 2      ▷ avoids integer overflow
4.      IF arr[mid].passNo == target   THEN  RETURN mid
5.      IF arr[mid].passNo  < target   THEN  lo ← mid + 1
6.      ELSE                                 hi ← mid - 1
7.  END WHILE
8.  RETURN -1""")

    heading2(doc, "6.6  AI Demand Heuristic — Gaussian PeakScore")
    body(doc,
        "peak_score(h) models urban parking demand across 24 hours by summing two "
        "Gaussian bell curves that represent the morning rush (centred at 09:00, "
        "σ ≈ 1.4 hr) and the evening rush (centred at 18:30, σ ≈ 1.3 hr), plus a "
        "flat baseline for midday occupancy (10:00–17:00). The result is clamped "
        "to [0.0, 1.0] before being mapped to a pricing multiplier.")
    add_code(doc,
"""Algorithm: PEAK_SCORE(hour h)
──────────────────────────────────────────────────────
morning_component ← 0.85 × exp( −0.5 × ((h − 9.0)  / 1.4)² )
evening_component ← 0.94 × exp( −0.5 × ((h − 18.5) / 1.3)² )
daytime_base      ← 0.62   if  10 ≤ h ≤ 17,  else  0.0
score             ← morning_component + evening_component + daytime_base
RETURN CLAMP(score, 0.0, 1.0)

Algorithm: PRICING_MULT(score)
──────────────────────────────────────────────────────
IF   score ≥ 0.80  →  RETURN 1.60   ▷ Peak surge tariff
ELIF score ≥ 0.55  →  RETURN 1.25   ▷ Moderate traffic
ELIF score ≤ 0.20  →  RETURN 0.80   ▷ Off-peak rebate
ELSE               →  RETURN 1.00   ▷ Normal rate""")

    # ── SECTION 7: FLOWCHARTS ─────────────────────────────────────────────────
    heading1(doc, "7.  Process Flowcharts")

    heading2(doc, "7.1  Vehicle Check-In Flowchart")
    embed_image(doc, IMG_FLW, "Vehicle Check-In Process — vehicle_arrive()", width=5.0)

    heading2(doc, "7.2  Vehicle Check-Out Flowchart (ASCII)")
    add_code(doc,
"""       vehicle_depart(tag, exitEpoch)
                    │
       ┌────────────▼────────────┐
       │  Linear scan bay_grid[] │
       │  to locate 'tag'        │
       └────────────┬────────────┘
                    │
          ┌─────────▼──────────┐
          │   Bay found?       │
          └──────┬─────────────┘
          NO ────┤           YES
                 │             │
          Print  │   ┌─────────▼──────────────────┐
          warning│   │ Compute billable hours,      │
          RETURN │   │ call PEAK_SCORE(hr),         │
                 │   │ compute fee = rate×hrs×mult  │
                 │   └─────────┬──────────────────┘
                 │             │
                 │   ┌─────────▼──────────┐
                 │   │ Prepend ExitLog     │
                 │   │ node to ledger list │
                 │   └─────────┬──────────┘
                 │             │
                 │   ┌─────────▼──────────┐
                 │   │ Print receipt,      │
                 │   │ release bay cell    │
                 │   └─────────┬──────────┘
                 │             │
                 │    ┌────────▼──────────────┐
                 │    │  hold_line.size > 0    │
                 │    │  AND category matches? │
                 │    └───┬──────────────┬────┘
                 │       YES             NO
                 │        │              │
                 │  ┌─────▼──────┐       │
                 │  │ pop_hold() │       │
                 │  │ assign_bay │       │
                 │  └─────┬──────┘       │
                 │        └──────────────┘
                 │                       │
                 └───────────────────────┘
                               │
                             END""")

    # ── SECTION 8: COMPLEXITY ANALYSIS ───────────────────────────────────────
    heading1(doc, "8.  Time & Space Complexity Analysis")

    heading2(doc, "8.1  Operation-Level Complexity")
    body(doc,
        "Let n = BAY_COUNT (20 bays),  m = number of completed sessions stored in "
        "the ExitLog linked list,  q = number of vehicles in the HoldLine queue.")

    rows_cplx = [
        ["vehicle_arrive() — Bay Scan",    "Linear Search",    "O(n)",       "O(1)"],
        ["vehicle_arrive() — Duplicate Check","Linear Search", "O(n)",       "O(1)"],
        ["vehicle_depart() — Locate Bay",  "Linear Search",    "O(n)",       "O(1)"],
        ["vehicle_depart() — Bill Record", "List Prepend",     "O(1)",       "O(1)"],
        ["vehicle_depart() — Queue Check", "Head Peek",        "O(1)",       "O(1)"],
        ["find_vehicle()",                 "Linear Search",    "O(n)",       "O(1)"],
        ["push_hold()",                    "Tail Insert",      "O(1)",       "O(1)"],
        ["pop_hold()",                     "Head Remove",      "O(1)",       "O(1)"],
        ["show_sorted_sessions()",         "Insertion Sort",   "O(m²)",      "O(m)"],
        ["lookup_pass()",                  "Sort + Bin Search","O(m log m)", "O(m)"],
        ["peak_score()",                   "Gaussian Formula", "O(1)",       "O(1)"],
        ["pricing_mult()",                 "Threshold Check",  "O(1)",       "O(1)"],
    ]
    info_table(doc,
        ["Operation", "Algorithm / Technique", "Time Complexity", "Space Complexity"],
        rows_cplx,
        col_widths=[2.6, 1.8, 1.3, 1.3])

    heading2(doc, "8.2  Overall Space Complexity")
    rows_space = [
        ["bay_grid[]",     "Static Array",     "O(n)",   "Fixed 20 × sizeof(ParkBay)  ≈ 900 bytes"],
        ["ExitLog ledger", "Linked List",       "O(m)",   "Grows by one node per departure"],
        ["HoldLine",       "FIFO Queue",        "O(q)",   "Bounded by number of waiting vehicles"],
        ["Temp sort array","Dynamic (malloc)",  "O(m)",   "Freed immediately after sort/display"],
        ["Total",          "—",                "O(n+m+q)","Dominated by session history (m)"],
    ]
    info_table(doc,
        ["Structure", "Type", "Space", "Notes"],
        rows_space,
        col_widths=[1.6, 1.5, 1.0, 2.9])

    heading2(doc, "8.3  Why These Choices Are Appropriate")
    body(doc,
        "BAY_COUNT (n) is fixed at 20. For such a small n, O(n) linear scans complete "
        "in microseconds on any modern processor — a hash-map or BST would add code "
        "complexity without measurable performance gain. Similarly, the session count m "
        "is bounded in practice (a parking lot turns over hundreds, not millions of "
        "vehicles per day), so Insertion Sort's O(m²) worst case is entirely acceptable "
        "and its stability is a practical benefit.")
    body(doc,
        "Binary search (O(log m)) is introduced for the pass-number lookup to demonstrate "
        "algorithmic contrast: the same data can be queried in O(m) with a linear scan or "
        "O(log m) after a one-time sort, illustrating the trade-off between preprocessing "
        "cost and query speed.")

    # ── SECTION 9: MODULE REFERENCE ───────────────────────────────────────────
    heading1(doc, "9.  Function Reference")
    rows_fn = [
        ["setup_bay_grid()",       "Initialises the bay_grid[] array, resets HoldLine."],
        ["vehicle_arrive()",       "Checks for duplicates → assigns bay or enqueues vehicle."],
        ["vehicle_depart()",       "Locates bay → computes fee → prepends ExitLog → frees bay → auto-queues."],
        ["assign_bay()",           "Linear scan of bay_grid[] to allocate the first free matching bay."],
        ["release_bay()",          "Clears a bay cell back to BAY_FREE state."],
        ["find_vehicle()",         "Linear search for a vehicle tag across all occupied bays."],
        ["push_hold()",            "Appends a CarInfo node to the TAIL of HoldLine (O(1))."],
        ["pop_hold()",             "Removes and returns the HEAD node of HoldLine (O(1))."],
        ["peak_score(hour)",       "Returns Gaussian demand score in [0.0, 1.0] for a given hour."],
        ["pricing_mult(score)",    "Maps demand score to a tariff multiplier (0.80× – 1.60×)."],
        ["show_sorted_sessions()", "Extracts ExitLog list → Insertion Sort → prints ranked table."],
        ["lookup_pass(pass)",      "Sorts by passNo → Binary Search → prints session record."],
        ["show_forecast()",        "Loops h=0..23, calls peak_score & pricing_mult, draws bar chart."],
        ["show_bay_layout()",      "Prints a 2-row grid map of all bays with FREE/BUSY status."],
        ["show_summary()",         "Prints total bays, occupancy, queue size, revenue."],
        ["run_demo()",             "Simulates 10 arrivals (Indian plates), search, exits, all reports."],
        ["same_tag(a, b)",         "Case-insensitive string comparison without stricmp (portable)."],
        ["epoch_to_str(ep)",       "Converts unix timestamp to 'DD-Mon HH:MM' display string."],
    ]
    info_table(doc, ["Function", "Description"], rows_fn, col_widths=[2.3, 3.9])

    # ── SECTION 10: SAMPLE OUTPUT ─────────────────────────────────────────────
    heading1(doc, "10.  Sample Execution Output (Demo Mode)")
    body(doc, "The following output is captured from:  ./smart_parking.exe --demo")

    exe = os.path.join(REPO, "smart_parking.exe")
    demo_out = ""
    if os.path.exists(exe):
        res = subprocess.run([exe, "--demo"], capture_output=True, text=True)
        demo_out = res.stdout[:4000]  # truncate very long outputs
        if len(res.stdout) > 4000:
            demo_out += "\n... [output truncated for brevity] ..."

    add_code(doc, demo_out if demo_out else "(executable not found)", font_size=8.0)

    # ── SECTION 11: CONCLUSION ────────────────────────────────────────────────
    heading1(doc, "11.  Conclusion")
    body(doc,
        "BayTrack successfully demonstrates that a practical, feature-complete parking "
        "management system can be built in under 500 lines of C code by choosing data "
        "structures and algorithms that are precisely matched to the problem constraints:")
    bullet(doc, [
        "A static array provides O(1) indexed access for a fixed-size bay grid.",
        "A FIFO linked queue guarantees first-come-first-served fairness with O(1) "
        "enqueue and dequeue.",
        "A singly linked list accumulates billing records dynamically without "
        "pre-allocation.",
        "Insertion sort and binary search illustrate the classic preprocessing vs. "
        "query-time trade-off.",
        "The Gaussian PeakScore AI model produces a smooth, realistic demand curve "
        "without external libraries, enabling dynamic pricing that responds to time-of-day.",
    ])
    body(doc,
        "The system is designed for straightforward extension: adding a third floor "
        "requires changing one constant; integrating persistent storage requires "
        "serialising the linked list to a flat file; introducing real sensor data "
        "requires replacing the heuristic demand curve with an empirically measured "
        "one. These properties make BayTrack a strong foundation for a production-ready "
        "parking system.")

    # ── SECTION 12: REFERENCES ────────────────────────────────────────────────
    heading1(doc, "12.  References")
    refs = [
        "[1]  Cormen, T. H., Leiserson, C. E., Rivest, R. L., & Stein, C. (2022). "
        "Introduction to Algorithms (4th ed.). MIT Press.",
        "[2]  Kernighan, B. W., & Ritchie, D. M. (1988). The C Programming Language "
        "(2nd ed.). Prentice Hall.",
        "[3]  Knuth, D. E. (1998). The Art of Computer Programming, Vol. 3: Sorting "
        "and Searching (2nd ed.). Addison-Wesley.",
        "[4]  ISO/IEC 9899:1999 — Programming Languages — C (C99 Standard). "
        "International Organization for Standardization.",
        "[5]  Ministry of Road Transport & Highways, Government of India — "
        "Vehicle Registration Format Guidelines. https://morth.nic.in/",
    ]
    for ref in refs:
        p = doc.add_paragraph(ref)
        p.paragraph_format.space_after  = Pt(4)
        p.paragraph_format.line_spacing = 1.2
        for run in p.runs:
            run.font.name = "Calibri"
            run.font.size = Pt(9.5)
            run.font.color.rgb = C_SLATE

    doc.save(OUT)
    print(f"Report saved: {OUT}")

if __name__ == "__main__":
    build_document()
