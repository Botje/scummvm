meta:
  id: kq8_ppl
  file-extension: ppl
  endian: le
seq:
- id: tag
  type: tag
- size: 4
- id: chunk
  type: chunk
  repeat: eos

types:
  tag:
    seq:
      - id: tag
        type: str
        size: 4
        encoding: ASCII
  chunk:
    seq:
      - id: tag
        type: str
        size: 4
        encoding: ASCII
      - id: len
        type: u4
      - id: val
        size: len # seems to be RGB0 for the data chunk