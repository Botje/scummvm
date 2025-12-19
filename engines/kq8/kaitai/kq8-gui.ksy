meta:
  id: kq8_gui
  file-extension: gui
  endian: le
seq:
  - id: tag
    size: 4
    type: str
    encoding: ASCII
  - id: len
    type: u4
  - id: version
    type: u4
  - id: body
    size: len - 4
    type:
      switch-on: tag
      cases:
        '"UIDL"': uidl
        '"CCBD"': ccbd
types:
  dialog:
    seq:
      - id: tag
        size: 4
        type: str
        encoding: ASCII
      - id: len
        type: u4
      - id: version
        type: u4
      - id: val
        type:
          switch-on: tag
          cases:
            '"UIDL"': uidl
            '"CCBD"': ccbd
  control:
    seq:
      - id: tag
        size: 4
        type: str
        encoding: ASCII
      - id: len
        type: u4
      - id: version
        type: u4
      - id: control
        type: control_shared
      - id: val
        type:
          switch-on: tag
          cases:
            '"UIBM"': uibm
            '"UITX"': uitx
            '"CCBB"': ccbb
  uibm:
    seq:
      - id: bitmap_tag
        type: s4
      - id: modifier
        type: u4
      - id: attribute
        type: u4
      - id: reserved
        type: u4
  uitx:
    seq:
      - id: font_tag
        type: s4
      - id: justification
        type: u4
      - id: text_tag
        type: s4
  ccbb:
    seq:
      - id: uitx
        type: uitx
      - id: bitmap_array_tag
        type: s4
      - id: modifier
        type: u4
      - id: attribute
        type: u4
      - id: reserved
        type: u4
  uidl:
    seq:
      - id: control
        type: control_shared
      - id: num_dialogs
        type: u4
      - id: num_controls
        type: u4
      - id: dialogs
        type: dialog
        repeat: expr
        repeat-expr: num_dialogs
      - id: controls
        type: control
        repeat: expr
        repeat-expr: num_controls
  ccbd:
    seq:
      - id: uidl
        type: uidl
      - id: caption_y
        type: s4
      - id: font_tag
        type: s4
      - id: justification
        type: u4
      - id: text_tag
        type: s4
      - id: bitmap_tag
        type: s4
      - id: modifier
        type: u4
      - id: attribute
        type: u4
      - id: reserved
        type: u4
  rect:
    seq:
      - id: left
        type: u4
      - id: top
        type: u4
      - id: right
        type: u4
      - id: bottom
        type: u4
    instances:
      width:
        value: right - left
      height:
        value: bottom - top
  control_shared:
    seq:
      - id: id
        type: u4
      - id: flags
        type: u4
      - id: rect
        type: rect