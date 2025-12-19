meta:
  id: kq8_dml
  file-extension: dml
  endian: le
seq:
  - id: tag
    contents: "PERS"
  - id: len
    type: u4
  - id: len2
    type: u2
  - size: len2
    contents: "TS::MaterialList"
  - id: version
    type: u4
  - type: u4
  - id: num_materials
    type: u4
  - id: materials
    type: material
    repeat: expr
    repeat-expr: num_materials
types:
  material:
    seq:
      - id: flags
        type: u4
      - id: alpha
        type: f4
      - id: index
        type: u4
      - id: r
        type: u1
      - id: g
        type: u1
      - id: b
        type: u1
      - size: 1
      - id: name
        size: "_parent.version < 2 ? 16 : 32"
        type: str
        terminator: 0
        encoding: ASCII
