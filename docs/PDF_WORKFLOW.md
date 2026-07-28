# PDF → Markdown workflow

## Purpose

Use this workflow to turn datasheets, schematics, and pinout PDFs into project files that Claude Code and Codex can both read.

## Folders

- `docs/source-pdf/`: original PDFs.
- `docs/extracted/<document>/`: generated Markdown and selected rendered page images.
- `docs/hardware/`: human-reviewed facts such as pinout, wiring, and power-tree notes.

## Extract a PDF

From the project root:

```powershell
python tools/extract_pdf.py "docs/source-pdf/board-pinout.pdf"
```

This creates a Markdown file under `docs/extracted/board-pinout/`.

## Extract text and render schematic pages

Use page numbers from the original PDF. For example, render pages 1, 3, 4, 5, and 8 at 180 DPI:

```powershell
python tools/extract_pdf.py "docs/source-pdf/board-schematic.pdf" --render-pages "1,3-5,8" --dpi 180
```

Rendered PNG files are written to `docs/extracted/<document>/images/`.

## Ask an AI to make reviewed notes

After extraction, use this prompt:

```text
Read the original PDF and the generated files under docs/extracted/<document>/.
Create docs/hardware/<topic>.md with only verified facts. Include original PDF page numbers.
Mark ambiguous text, diagrams, or pin labels as "待确认". Do not modify .syscfg, generated files, or CCS project files.
```

## MSPM0 project rule

Markdown extraction is a convenience layer, not a source of truth. For pin assignments, voltage levels, timing, or debug wiring, always verify against the original PDF before editing `.syscfg` or connecting hardware.