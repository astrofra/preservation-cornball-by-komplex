# Cutter Pass 05

Date: 2026-07-12
Tooling: Cutter package `2.5.0`, using the bundled `rizin.exe`
Binary: `reverse/baseline/cornball/PLANET.EXE`

Related artifacts:

- [planet-scene-helper-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-helper-map.csv)
- [planet-geometry-helpers.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-geometry-helpers.csv)
- [planet_first_pass.rz](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/planet_first_pass.rz)

## Goal

Resolve the exact geometry emitted by the two most reusable scene helpers:

- `0x00403760`
- `0x00403e40`

## `draw_dual_texture_bipyramid` at `0x00403760`

This helper was previously labeled as a generic panel pair. The geometry is now clear enough to correct that.

### Confirmed shape

- OpenGL primitive: `GL_TRIANGLES`
- Total vertices: `24`
- Total triangles: `8`
- Parameters: two texture IDs

The helper draws two square pyramids that share the same base square at `z = 0`:

- base corners: `(-60, -60, 0)`, `(60, -60, 0)`, `(60, 60, 0)`, `(-60, 60, 0)`
- front apex: `(0, 0, 60)`
- back apex: `(0, 0, -60)`

This is a textured square bipyramid or octahedron-like sprite, not two flat panels.

### Texture usage

The first texture is bound and used on the `+z` half.

The second texture is bound and used on the `-z` half.

Both halves use the same texture pattern:

- apex mapped near `(0.5, 0.5)`
- base corners mapped to the outer texture square

### Equivalent pseudocode

```c
void draw_dual_texture_bipyramid(GLuint front_tex, GLuint back_tex) {
    glBegin(GL_TRIANGLES);

    glBindTexture(GL_TEXTURE_2D, front_tex);
    emit_pyramid(apex_front, base_square_z0);

    glBindTexture(GL_TEXTURE_2D, back_tex);
    emit_pyramid(apex_back, base_square_z0);

    glEnd();
}
```

## `draw_cached_tube_shell` at `0x00403e40`

This helper was previously described as a ring strip. The cache and draw order now show a more precise shape.

### Cache build

On first use, the helper fills two arrays:

- [tube_ring_vertices_neg_z](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/planet_first_pass.rz)
- [tube_ring_vertices_pos_z](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/planet_first_pass.rz)

Each array contains `64` vertices of three floats each.

Resolved build formula:

```c
for (int ring = 0; ring < 2; ++ring) {
    float z = (ring - 0.5f) * 140.0f;   // -70, +70
    for (int i = 0; i < 64; ++i) {
        float angle = i * 0.098125f;
        verts[ring][i].x = cosf(angle) * 6.0f;
        verts[ring][i].y = sinf(angle) * 6.0f;
        verts[ring][i].z = z;
    }
}
```

This is a pair of cached circles with:

- radius `6`
- ring planes at `z = -70` and `z = +70`
- `64` samples per ring

### Draw phase

- OpenGL primitive: `GL_QUADS`
- Total quads: `64`
- Total submitted vertices: `256`
- Parameter: one `double phase`

The helper connects matching vertices from the two circles into a cylindrical shell:

```c
for (int i = 0; i < 64; ++i) {
    int next = (i + 1) & 63;
    emit_quad(
        ring0[i],
        ring0[next],
        ring1[next],
        ring1[i]
    );
}
```

### Texture-coordinate behavior

The `phase` argument becomes a scrolling texture window:

- one side of the tube uses `phase`
- the opposite side uses `phase + 3.0`
- the around-the-ring coordinate advances in `1/64` steps

So the helper is best treated as a time-scrolled tube shell rather than a decorative ring outline.

### GL state

Before drawing, the helper:

- enables `GL_FOG`
- disables `GL_CULL_FACE`

After drawing, it restores `GL_CULL_FACE`.

## Caller-side interpretation

The two confirmed callers pass different phase values derived from `g_scene_elapsed_seconds`, so the same tube shell geometry is reused with different texture-scroll speeds in the `kaar` and `surf` families.

## Outcome

Pass 05 tightens two important reconstruction targets:

- the former “panel pair” is now a precise textured bipyramid helper
- the former “ring strip” is now a precise cached tube-shell helper

This is a materially better base for C or C++ source reconstruction because the helper boundaries now correspond to concrete primitives instead of vague visual guesses.
