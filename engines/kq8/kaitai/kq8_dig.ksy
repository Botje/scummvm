meta:
  id: kq8_dig
  file-extension: dig
  endian: le
seq:
  - contents: "PERS"
  - size: 6
  - contents: "ITRGeometry\0"
  - id: version
    type: u4
  - type: u4
  - id: texture_scale
    type: f4
  - id: min_bounds
    type: vec3
  - id: max_bounds
    type: vec3
  - id: num_surfaces
    type: u4
  - id: num_bsp_nodes
    type: u4
  - id: num_bsp_leafs
    type: u4
  - id: blob_size
    type: u4
  - id: num_verts
    type: u4
  - id: num_points
    type: u4
  - id: num_texcoords
    type: u4
  - id: num_planes
    type: u4
  - id: surfaces
    type: surface
    repeat: expr
    repeat-expr: num_surfaces
  - id: bsp_nodes
    type: bsp_node
    repeat: expr
    repeat-expr: num_bsp_nodes
  - id: bsp_leafs
    type: bsp_leaf
    size: 48
    repeat: expr
    repeat-expr: num_bsp_leafs
  - id: global_blob
    size: blob_size
  - id: verts
    type: vert
    repeat: expr
    repeat-expr: num_verts
  - id: points
    type: vec3
    repeat: expr
    repeat-expr: num_points
  - id: texcoords
    type: vec2
    repeat: expr
    repeat-expr: num_texcoords
  - id: planes
    type: plane
    repeat: expr
    repeat-expr: num_planes
  - id: highest_mip
    type: s4
  - id: flags
    type: u4
types:
  vert:
    seq:
      - id: p_idx
        type: u2
      - id: t_idx
        type: u2
  vec2:
    seq:
      - id: x
        type: f4
      - id: y
        type: f4
  vec3:
    seq:
      - id: x
        type: f4
      - id: y
        type: f4
      - id: z
        type: f4
  surface:
    seq:
    - id: flags
      type: u1
    - id: materials
      type: u1
    - id: tsx
      type: u1
    - id: tsy
      type: u1
    - id: tox
      type: u1
    - id: toy
      type: u1
    - id: plane_idx
      type: u2
    - id: vert_idx
      type: u4
    - id: point_idx
      type: u4
    - id: num_verts
      type: u1
    - id: num_points
      type: u1
    - id: pad
      size: 2
  plane:
    seq:
      - id: x
        type: f4
      - id: y
        type: f4
      - id: z
        type: f4
      - id: d
        type: f4
  bsp_node:
    seq:
      - id: plane_idx
        type: u2
      - id: front
        type: s2
      - id: back
        type: s2
      - id: fill
        type: s2
  bsp_leaf:
    seq:
      # leaf_type == 2 implies num_a == num_b == offset_a == offset_b
      - id: leaf_type
        type: u2
        enum: leaf_type
      # Refers to global_blob
      - id: pvs_num
        type: u2
      - id: pvs_offset
        type: u4
      - id: num_surfaces
        type: u2
      - id: num_planes
        type: u2
      # These refer to global_blob
      - id: num_a
        type: u2
      - id: num_b
        type: u2
      - id: offset_a
        type: u4
      - id: offset_b
        type: u4
      - id: min_bounds
        type: vec3
      - id: max_bounds
        type: vec3
enums:
  leaf_type:
    1: outside