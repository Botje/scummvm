meta:
  id: kq8_pft
  file-extension: pft
  endian: le
seq:
  - id: tag
    contents: "PFON"
  - id: len
    type: u4
  - id: brol
    size: 12
  - id: num_glyphs
    type: u4
  - id: max_height
    type: u4
  - id: max_width
    type: u4
  - id: brol2
    type: u4
    repeat: expr
    repeat-expr: 6
  - id: alphabet_size
    type: u4
  - id: mapping
    type: s2
    repeat: expr
    repeat-expr: alphabet_size
  - id: more_brol
    type: u4
    repeat: expr
    repeat-expr: num_glyphs * 2
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
    - type: u4
    - id: rmaps
      type: u4
      repeat: expr
      repeat-expr: num_bitmaps
    - id: bitmaps
      type: bitmap
      repeat: expr
      repeat-expr: num_bitmaps

  bitmap:
    seq:
    - id: tag
      contents: "PBMP"
    - id: total_len
      type: u4
    - id: chunks
      type: bitmap_chunk
    instances:
      width:
        value: chunks.width
      height:
        value: chunks.height
  bitmap_chunk:
    seq:
    - id: head_tag
      contents: "head"
    - id: head_len
      type: u4
      valid: 0x14
    - type: u4
      valid: 2
    - id: width
      type: u4
    - id: height
      type: u4
    - type: u4
      valid: 8
    - type: u4
    - id: data_tag
      contents: "data"
    - id: data_len
      type: u4
    - id: data_body
      size: data_len
      doc: contains `height` * `4 * ceil(width/4)` pixels, of which `width` are used
    - id: detl_tag
      contents: "DETL"
    - id: detl_len
      type: u4
    - size: detl_len