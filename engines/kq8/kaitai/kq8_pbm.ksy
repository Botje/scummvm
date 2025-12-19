meta:
  id: kq8_pbm
  file-extension: pbm
  endian: le
seq:
  - id: tag
    type: str
    size: 4
    encoding: ASCII
  - size: 4
  - id: head_tag
    type: tag
  - id: head_len
    type: u4
  - id: brol
    type: u4
    repeat: expr
    repeat-expr: head_len / 4
  - id: data_tag
    type: tag
  - id: data_len
    type: u4
  - id: data_body
    size: data_len
    doc: contains `height` * roundUpTo4(`width`) pixels
# there's a DETL after this occasionally but we have no idea what it's for.
instances:
  width:
    value: brol[1]
  height:
    value: brol[2]
types:
  tag:
    seq:
      - id: tag
        type: str
        size: 4
        encoding: ASCII