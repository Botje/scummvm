meta:
  id: kq8_msg
  file-extension: msg
  endian: le

seq:
  - id: res_type
    type: u1
  - type: u1
  - id: version
    type: u4
  - id: data_size
    type: u2
  - id: last_id
    type: u2
  - id: count
    type: u2
  - id: messages
    type: message
    repeat: expr
    repeat-expr: count
  - id: text
    type: strz
    repeat: expr
    repeat-expr: count
    encoding: ascii
  - id: comments
    type: comment
    repeat: expr
    repeat-expr: count

types:
  message:
    seq:
    - id: noun
      type: u1
    - id: verb
      type: u1
    - id: case
      type: u1
    - id: sequence
      type: u1
    - id: talker
      type: u1
    - id: text_offset
      type: u2
    - id: ref_noun
      type: u1
    - id: ref_verb
      type: u1
    - id: ref_case
      type: u1
    - id: ref_sequence
      type: u1
    instances:
      text:
        io: _root._io
        pos: text_offset+2
        type: strz
        encoding: ascii
  comment:
    seq:
    - id: comment
      type: strz
      encoding: ascii
    - type: u4
    - type: u2