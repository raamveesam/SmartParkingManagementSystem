import os
import docx
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml import OxmlElement, parse_xml
from docx.oxml.ns import nsdecls, qn

def set_cell_background(cell, fill_hex):
    tcPr = cell._tc.get_or_add_tcPr()
    shd = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{fill_hex}"/>')
    tcPr.append(shd)

def set_cell_margins(cell, top=100, bottom=100, left=150, right=150):
    tcPr = cell._tc.get_or_add_tcPr()
    tcMar = parse_xml(
        f'<w:tcMar {nsdecls("w")}>'
        f'<w:top w:w="{top}" w:type="dxa"/>'
        f'<w:bottom w:w="{bottom}" w:type="dxa"/>'
        f'<w:left w:w="{left}" w:type="dxa"/>'
        f'<w:right w:w="{right}" w:type="dxa"/>'
        f'</w:tcMar>'
    )
    tcPr.append(tcMar)

def add_code_block(doc, code_text):
    tbl = doc.add_table(rows=1, cols=1)
    tbl.alignment = WD_TABLE_ALIGNMENT.CENTER
    tbl.autofit = False
    
    cell = tbl.cell(0, 0)
    cell.width = Inches(6.5)
    set_cell_background(cell, "F4F5F7")
    set_cell_margins(cell, top=120, bottom=120, left=180, right=180)
    
    # Border styling
    tcPr = cell._tc.get_or_add_tcPr()
    borders = parse_xml(
        f'<w:tcBorders {nsdecls("w")}>'
        f'<w:top w:val="single" w:sz="4" w:space="0" w:color="D1D5DB"/>'
        f'<w:left w:val="single" w:sz="18" w:space="0" w:color="2563EB"/>'
        f'<w:bottom w:val="single" w:sz="4" w:space="0" w:color="D1D5DB"/>'
        f'<w:right w:val="single" w:sz="4" w:space="0" w:color="D1D5DB"/>'
        f'</w:tcBorders>'
    )
    tcPr.append(borders)
    
    p = cell.paragraphs[0]
    p.paragraph_format.space_before = Pt(2)
    p.paragraph_format.space_after = Pt(2)
    p.paragraph_format.line_spacing = 1.15
    
    run = p.add_run(code_text)
    run.font.name = 'Consolas'
    run.font.size = Pt(8.5)
    run.font.color.rgb = RGBColor(30, 41, 59)
    
    # Add spacing after table
    spacer = doc.add_paragraph()
    spacer.paragraph_format.space_before = Pt(4)
    spacer.paragraph_format.space_after = Pt(8)

def main():
    repo_dir = r"C:\Users\RamLaxma\.gemini\antigravity\scratch\SmartParkingManagementSystem"
    output_docx = os.path.join(repo_dir, "Smart_Parking_Management_System_Code.docx")
    
    doc = Document()
    
    # Page setup - 1 inch margins
    sections = doc.sections
    for section in sections:
        section.top_margin = Inches(1.0)
        section.bottom_margin = Inches(1.0)
        section.left_margin = Inches(1.0)
        section.right_margin = Inches(1.0)
        
    # Title Page
    title_p = doc.add_paragraph()
    title_p.paragraph_format.space_before = Pt(36)
    title_p.paragraph_format.space_after = Pt(12)
    title_p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run_title = title_p.add_run("SMART PARKING MANAGEMENT SYSTEM")
    run_title.font.name = 'Arial'
    run_title.font.size = Pt(24)
    run_title.font.bold = True
    run_title.font.color.rgb = RGBColor(30, 58, 138)
    
    sub_p = doc.add_paragraph()
    sub_p.paragraph_format.space_after = Pt(24)
    sub_p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run_sub = sub_p.add_run("Data Structures, Algorithms (DSA) & AI Demand Forecasting in C (C99)\nGitHub Repository: raamveesam/SmartParkingManagementSystem")
    run_sub.font.name = 'Arial'
    run_sub.font.size = Pt(12)
    run_sub.font.italic = True
    run_sub.font.color.rgb = RGBColor(100, 116, 139)
    
    # Abstract Box
    h_abs = doc.add_heading("Project Abstract & Overview", level=1)
    h_abs.paragraph_format.space_before = Pt(16)
    h_abs.paragraph_format.space_after = Pt(8)
    
    abs_text = (
        "The Smart Parking Management System is an end-to-end multi-floor parking automation infrastructure "
        "implemented in standard C (C99). It integrates foundational Data Structures and Algorithms (DSA) with "
        "an Artificial Intelligence demand forecasting extension:\n\n"
        "• Arrays: Real-time 3-floor parking layout grid (30 slots) categorized for 2-wheelers, 4-wheelers, and heavy vehicles.\n"
        "• Linked Lists: Dynamic Vehicle Registry database supporting O(1) insertions, and historical billing transactions log.\n"
        "• FIFO Queues: Overflow traffic waiting line that automatically enqueues arriving vehicles when capacity is saturated, "
        "and automatically dequeues and allocates slots when parked vehicles exit.\n"
        "• Searching & Sorting: Linear search for active vehicles, Binary search for ticket queries, Quick Sort for revenue rankings, "
        "and Merge Sort for chronological session ordering.\n"
        "• AI Demand Prediction: Combines an urban diurnal traffic curve prior with Ordinary Least Squares (OLS) regression to forecast "
        "hourly occupancy and compute dynamic surge pricing (0.85x off-peak to 1.50x peak rush)."
    )
    p_abs = doc.add_paragraph(abs_text)
    p_abs.paragraph_format.line_spacing = 1.25
    p_abs.paragraph_format.space_after = Pt(18)
    
    doc.add_page_break()
    
    # File listing in order of presentation
    files_to_include = [
        ("Architecture & Usage Guide", "README.md"),
        ("Build Configuration", "Makefile"),
        ("Header File - Data Structures & Function Prototypes", "include/parking_system.h"),
        ("Entry Point & Interactive CLI Interface", "src/main.c"),
        ("Slot Manager (Arrays & 2D Grid Visualizer)", "src/slot_manager.c"),
        ("Vehicle Registry (Linked List)", "src/vehicle_registry.c"),
        ("Waiting Queue (FIFO Queue)", "src/waiting_queue.c"),
        ("Billing & Invoicing (Linked List)", "src/billing.c"),
        ("Vehicle Entry & Exit Workflow", "src/entry_exit.c"),
        ("Searching & Sorting Algorithms (DSA)", "src/search_sort.c"),
        ("AI Demand Prediction & Dynamic Tariff Engine", "src/ai_predictor.c"),
        ("Facility Operations Analytics & Statistics", "src/statistics.c"),
        ("System Persistence & State File I/O", "src/file_io.c"),
        ("Automated End-to-End Simulation Demo", "src/demo.c"),
        ("Formatting Utilities & Case-Insensitive String Match", "src/utils.c")
    ]
    
    for title, rel_path in files_to_include:
        full_path = os.path.join(repo_dir, rel_path.replace("/", os.sep))
        if not os.path.exists(full_path):
            continue
            
        with open(full_path, "r", encoding="utf-8", errors="replace") as f:
            code_content = f.read()
            
        h = doc.add_heading(f"{title} ({rel_path})", level=2)
        h.paragraph_format.space_before = Pt(14)
        h.paragraph_format.space_after = Pt(6)
        
        add_code_block(doc, code_content)
        
    # Final Output / Demo Run Section
    doc.add_page_break()
    h_out = doc.add_heading("System Execution Log (Automated Demo Simulation)", level=1)
    h_out.paragraph_format.space_before = Pt(16)
    h_out.paragraph_format.space_after = Pt(8)
    
    # Run the demo and capture output
    import subprocess
    exe_path = os.path.join(repo_dir, "smart_parking.exe")
    demo_output = ""
    if os.path.exists(exe_path):
        res = subprocess.run([exe_path, "--demo"], capture_output=True, text=True)
        demo_output = res.stdout
    
    if demo_output:
        add_code_block(doc, demo_output)
        
    doc.save(output_docx)
    print(f"Document successfully created at: {output_docx}")

if __name__ == "__main__":
    main()
