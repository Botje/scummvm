meta:
  id: kq8_kqs
  file-extension: kqs
  endian: le
  imports:
    - kq8_dml
seq:
  - contents: "PERS"
  - type: u4
  - id: tag_len
    type: u2
  - id: tag
    type: str
    size: tag_len + (tag_len & 1)
    encoding: ASCII
    terminator: 0
  - id: version
    type: u4
  - id: bounding_sphere1
    type: bounding_sphere
  - id: num_nodes
    type: u4
  - id: num_sequences
    type: u4
  - id: a_48
    type: u4
  - id: a_52
    type: u4
  - id: a_56
    type: u4
  - id: num_names
    type: u4
  - id: num_meshes
    type: u4
  - id: num_transitions
    type: u4
    if: version >= 2
  - id: nodes
    size: 60
    type: node
    repeat: expr
    repeat-expr: num_nodes
  - id: sequences
    type: sequence
    repeat: expr
    repeat-expr: num_sequences
  - id: arr_48
    size: 56
    repeat: expr
    repeat-expr: a_48
    type: t_48
  - id: stuff
    type: vec3
  - id: stuff_from_a52
    type: u4
    repeat: expr
    repeat-expr: a_52
  - id: stuff2
    type: stuff2
  - id: names
    repeat: expr
    repeat-expr: num_names
    type: str
    size: 24
    encoding: ASCII
    terminator: 0
  - id: transitions
    repeat: expr
    repeat-expr: num_transitions
    type: transition
    if: version >= 2
  - id: meshes
    type: mesh
    repeat: expr
    repeat-expr: num_meshes
  - id: has_material
    type: u4
  - id: material_list
    type: kq8_dml
    if: has_material == 1
types:
  node:
    seq:
      - id: flags
        type: u4
      - id: rot_matrix
        type: f4
        repeat: expr
        repeat-expr: 9
      - id: offset
        type: vec3
      - id: mesh
        type: u2
      - id: frame
        type: u2
      - id: unk3
        type: u2
      - id: unk4
        type: u2
  vec3:
    seq:
      - id: x
        type: f4
      - id: y
        type: f4
      - id: z
        type: f4
  mesh:
    seq:
      - contents: "PERS"
      - id: size
        type: u4
      - id: mesh
        type: mesh_body
        size: size
  mesh_body:
    seq:
      - type: u2
      - contents: "TS::CelAnimMesh\0"
      - id: version
        type: u4
      - id: num_verts
        type: u4
      - id: verts_per_frame
        type: u4
      - id: num_texture_verts
        type: u4
      - id: num_faces
        type: u4
      - id: num_frames
        type: u4
      - id: texture_verts_per_frame
        type: u4
        if: version >= 2
      - id: scale
        type: vec3
        if: version < 3
      - id: origin
        type: vec3
        if: version < 3
      - id: radius
        type: f4
      - id: packed_vertices
        repeat: expr
        repeat-expr: num_verts
        type: packed_vertex
      - id: texture_vertices
        repeat: expr
        repeat-expr: num_texture_verts
        type: texcoord
      - id: faces
        repeat: expr
        repeat-expr: num_faces
        type: face
      - id: frames
        repeat: expr
        repeat-expr: num_frames
        type: frame
  packed_vertex:
    seq:
      - id: x
        type: u1
      - id: y
        type: u1
      - id: z
        type: u1
      - id: normal
        type: u1
  texcoord:
    seq:
      - id: u
        type: f4
      - id: v
        type: f4
  face:
    seq:
      - id: vert0_index
        type: u4
      - id: tex0_index
        type: u4
      - id: vert1_index
        type: u4
      - id: tex1_index
        type: u4
      - id: vert2_index
        type: u4
      - id: tex2_index
        type: u4
      - id: mat_index
        type: u4
  frame:
    seq:
      - id: first_vertex
        type: u4
      - id: scale
        type: vec3
        if: _parent.version >= 3
      - id: origin
        type: vec3
        if: _parent.version >= 3
  t_48:
    seq:
      - id: flags
        type: u4
      - id: rot_matrix
        type: f4
        repeat: expr
        repeat-expr: 9
      - id: offset
        type: vec3
      - id: unk1
        type: u2
      - id: unk2
        type: u2
  transition:
    seq:
      - id: name
        type: u2
      - id: flags
        type: u2
      - id: floats
        type: f4
        repeat: expr
        repeat-expr: 6
      - id: unknown
        type: f4
        repeat: expr
        repeat-expr: 2
        if: _parent.version >= 4
  sequence:
    seq:
      - id: node_index
        type: u2
      - id: node_count
        type: u2
  stuff2:
    seq:
      - id: unk2
        type: u2
      - id: unk3
        type: u2

  bounding_sphere:
    seq:
        - id: radius
          type: f4
        - id: center
          type: vec3