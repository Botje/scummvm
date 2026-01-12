meta:
  id: kq8_pft
  file-extension: kq8_pft
  endian: le
  imports:
  - kq8_pbm
seq:
  - id: tag
    contents: "PFON"
  - id: len
    type: u4
  - id: version
    type: u4
  - id: font_flags
    type: u4
  - id: text_flags
    type: u4
  - id: num_glyphs
    type: u4
  - id: max_height
    type: u4
  - id: max_width
    type: u4
  - id: text_color
    type: u4
  - id: back_color
    type: u4
  - id: base_line
    type: s4
  - id: brol2
    type: u4
    repeat: expr
    repeat-expr: 3
  - id: alphabet_size
    type: s2
  - id: alphabet_first
    type: s2
  - id: mapping
    type: s2
    repeat: expr
    repeat-expr: alphabet_size
  - id: glyph_infos
    type: glyph_info
    repeat: expr
    repeat-expr: num_glyphs
  - id: pbma
    type: pbma
  - type: u4
    valid: 0
types:
  tag:
    seq:
      - id: tag
        type: str
        size: 4
        encoding: ASCII
  pbma:
    seq:
    - id: pbma_tag
      contents: "PBMA"
    - id: pbma_len
      type: u4
    - id: head_tag
      contents: "head"
    - id: head_len
      type: u4
    - id: num_rmaps
      type: u4
    - id: num_bitmaps
      type: u4
    - id: rmap_tag
      contents: "rmap"
      if: num_rmaps > num_bitmaps
    - type: u4
      if: num_rmaps > num_bitmaps
    - id: rmaps
      type: u4
      repeat: expr
      repeat-expr: num_bitmaps
      if: num_rmaps > num_bitmaps
    - id: bitmaps
      type: kq8_pbm
      repeat: expr
      repeat-expr: num_bitmaps
  glyph_info:
    seq:
      - id: bitmap_index
        type: u1
      - id: bitmap_left
        type: u1
      - id: bitmap_top
        type: u1
      - id: width
        type: u1
      - id: height
        type: u1
      - id: baseline_shift
        type: s1
      - size: 2