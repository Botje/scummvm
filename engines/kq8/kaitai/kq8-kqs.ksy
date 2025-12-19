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
    if: tag == "KQ8::Shape"
  - id: num_nodes
    type: u4
  - id: num_subsequences
    type: u4
  - id: num_sequences
    type: u4
  - id: num_keyframes
    type: u4
  - id: num_loops
    type: u4
  - id: num_names
    type: u4
  - id: num_meshes
    type: u4
  - id: num_lightnodes
    type: u4
    if: version >= 2
  - id: nodes
    size: 60
    type: node
    repeat: expr
    repeat-expr: num_nodes
  - id: subsequences
    type: subsequence
    repeat: expr
    repeat-expr: num_subsequences
  - id: sequences
    size: 56
    repeat: expr
    repeat-expr: num_sequences
    type: sequence
  - id: keyframes
    type: f4
    repeat: expr
    repeat-expr: num_keyframes
  - id: loops
    type: loop
    repeat: expr
    repeat-expr: num_loops
  - id: names
    repeat: expr
    repeat-expr: num_names
    type: str
    size: 24
    encoding: ASCII
    terminator: 0
  - id: lightnodes
    repeat: expr
    repeat-expr: num_lightnodes
    type: lightnode
    if: version >= 2
  - size: 124
    if: tag == "TS::Shape"
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
      - id: parent_node
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
  sequence:
    seq:
      - id: flags
        type: u4
      - id: rot_matrix
        type: f4
        repeat: expr
        repeat-expr: 9
      - id: offset
        type: vec3
      - id: subsequence_index
        type: s2
      - id: light_index
        type: s2
  lightnode:
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
        if: _root.version >= 4
  subsequence:
    seq:
      - id: node_index
        type: u2
      - id: node_count
        type: u2
  loop:
    seq:
      - id: duration
        type: f4
      - id: name_index
        type: s2
      - id: keyframe_index
        type: s2
      - id: keyframe_count
        type: s2
      - id: sequence_index
        type: s2
      - id: sequence_count
        type: s2
      - size: 2

  bounding_sphere:
    seq:
        - id: radius
          type: f4
        - id: center
          type: vec3