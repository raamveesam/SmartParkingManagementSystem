import os
import subprocess
import docx
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml import parse_xml
from docx.oxml.ns import nsdecls

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
    set_cell_background(cell, "F8FAFC")
    set_cell_margins(cell, top=100, bottom=100, left=150, right=150)
    
    tcPr = cell._tc.get_or_add_tcPr()
    borders = parse_xml(
        f'<w:tcBorders {nsdecls("w")}>'
        f'<w:top w:val="single" w:sz="4" w:space="0" w:color="CBD5E1"/>'
        f'<w:left w:val="single" w:sz="18" w:space="0" w:color="1D4ED8"/>'
        f'<w:bottom w:val="single" w:sz="4" w:space="0" w:color="CBD5E1"/>'
        f'<w:right w:val="single" w:sz="4" w:space="0" w:color="CBD5E1"/>'
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
    
    spacer = doc.add_paragraph()
    spacer.paragraph_format.space_before = Pt(2)
    spacer.paragraph_format.space_after = Pt(6)

def main():
    repo_dir = r"C:\Users\RamLaxma\.gemini\antigravity\scratch\SmartParkingManagementSystem"
    output_docx = os.path.join(repo_dir, "Smart_Parking_Management_System_Code.docx")
    
    doc = Document()
    
    # 1-inch margins
    for section in doc.sections:
        section.top_margin = Inches(1.0)
        section.bottom_margin = Inches(1.0)
        section.left_margin = Inches(1.0)
        section.right_margin = Inches(1.0)
        
    # Title
    p_title = doc.add_paragraph()
    p_title.paragraph_format.space_before = Pt(24)
    p_title.paragraph_format.space_after = Pt(8)
    p_title.alignment = WD_ALIGN_PARAGRAPH.CENTER
    r_title = p_title.add_run("SMART PARKING MANAGEMENT SYSTEM")
    r_title.font.name = 'Arial'
    r_title.font.size = Pt(22)
    r_title.font.bold = True
    r_title.font.color.rgb = RGBColor(30, 58, 138)
    
    p_sub = doc.add_paragraph()
    p_sub.paragraph_format.space_after = Pt(18)
    p_sub.alignment = WD_ALIGN_PARAGRAPH.CENTER
    r_sub = p_sub.add_run("Data Structures, Algorithms (DSA) & AI Demand Forecasting in C (C99)\nGitHub: github.com/raamveesam/SmartParkingManagementSystem")
    r_sub.font.name = 'Arial'
    r_sub.font.size = Pt(11)
    r_sub.font.italic = True
    r_sub.font.color.rgb = RGBColor(100, 116, 139)
    
    # Abstract
    h_abs = doc.add_heading("1. Project Abstract & Objectives", level=1)
    h_abs.paragraph_format.space_before = Pt(12)
    h_abs.paragraph_format.space_after = Pt(6)
    
    abs_desc = (
        "This project implements a concise and practical Smart Parking Management System in C (C99 standard).\n\n"
        "Key Objectives & DSA Extensions:\n"
        "• Manage Parking Slots (Arrays): 20 slots across 2 floors categorized for 2-wheelers and 4-wheelers.\n"
        "• Manage Overflow Traffic (FIFO Queue): Waiting queue using linked nodes (front and rear pointers).\n"
        "• Historical Transactions (Linked List): Dynamic session log for duration and billing.\n"
        "• Searching & Sorting: Linear search by vehicle plate and sorting transactions by revenue.\n"
        "• AI Demand Prediction: Heuristic model that forecasts 24-hour urban congestion curves and computes dynamic surge pricing (0.85x to 1.50x).\n"
        "• Automatic Dispatch: When a parked car exits, the freed slot is immediately auto-allocated to the next queued vehicle."
    )
    p_a = doc.add_paragraph(abs_desc)
    p_a.paragraph_format.line_spacing = 1.2
    p_a.paragraph_format.space_after = Pt(12)
    
    # Source Code
    h_code = doc.add_heading("2. Complete Source Code (main.c)", level=1)
    h_code.paragraph_format.space_before = Pt(14)
    h_code.paragraph_format.space_after = Pt(6)
    
    main_c_path = os.path.join(repo_dir, "main.c")
    with open(main_c_path, "r", encoding="utf-8") as f:
        code_content = f.read()
    add_code_block(doc, code_content)
    
    # Execution Output
    h_out = doc.add_heading("3. Execution Output (Automated Test Simulation)", level=1)
    h_out.paragraph_format.space_before = Pt(14)
    h_out.paragraph_format.space_after = Pt(6)
    
    exe_path = os.path.join(repo_dir, "smart_parking.exe")
    demo_output = ""
    if os.path.exists(exe_path):
        res = subprocess.run([exe_path, "--demo"], capture_output=True, text=True)
        demo_output = res.stdout
    if demo_output:
        add_code_block(doc, demo_output)
        
    doc.save(output_docx)
    print(f"Document created successfully: {output_docx}")

if __name__ == "__main__":
    main()
