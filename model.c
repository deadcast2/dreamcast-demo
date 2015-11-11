#include <kos.h>
#include <GL/gl.h>
#include <tga/tga.h>
#include "model.h"

uint load_texture(char *filename)
{
    uint texID;
    kos_img_t img;
    pvr_ptr_t txr;

    if (tga_to_img(filename, &img) < 0)
    {
        printf("Could not load texture %s\n", filename);
        return 0;
    }

    txr = pvr_mem_malloc(img.w * img.h * 2);
    pvr_txr_load_kimg(&img, txr, PVR_TXRLOAD_INVERT_Y);
    kos_img_free(&img, 0);

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 img.w, img.h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_SHORT_4_4_4_4_TWID,
                 txr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FILTER, GL_FILTER_NONE);

    return texID;
}

wv_material *load_mat_file(char *filename, int *count)
{
    int mat_count = 0;
    char line[256];
    FILE *file = fopen(filename, "r");
    int i = 0;

    if (file == NULL)
    {
        printf("Error opening %s\n", filename);
        return NULL;
    }

    while (fgets(line, sizeof(line), file))
    {
        char mat_name[64];
        int result = sscanf(line, "newmtl %s", mat_name);
        if (result == 1) mat_count++;
    }

    wv_material *materials = malloc(sizeof(wv_material) * mat_count);

    rewind(file);

    int found_block = 1;
    char *last_found_mat_name;
    char mat_name[64], mat_filename[128];
    int result;
    wv_material material;

    while (fgets(line, sizeof(line), file))
    {
        result = sscanf(line, "newmtl %s", mat_name);
        if (result == 1)
        {
            if (found_block)
            {
                found_block = 0;
            }
            else
            {
                material.name = last_found_mat_name;
                material.texture_id = 0;
                materials[i] = material;
                i++;
                found_block = 1; // restart
            }

            last_found_mat_name = malloc(sizeof(mat_name));
            strcpy(last_found_mat_name, mat_name);
        }

        if (line[0] == 'K' && line[1] == 'd' && line[2] == ' ')
        {
            sscanf(line, "Kd %f %f %f",
                   &material.diffuse[0],
                   &material.diffuse[1],
                   &material.diffuse[2]);
        }

        result = sscanf(line, "map_Kd %s", mat_filename);
        if (result == 1)
        {
            material.name = last_found_mat_name;
            material.texture_id = load_texture(mat_filename);
            materials[i] = material;
            i++;
            found_block = 1; // restart
        }
    }

    fclose(file);

    *count = mat_count;
    return materials;
}

wv_model load(char *filename)
{
    char line[256];
    char mat_filename[64];
    int vertex_count = 0, face_count = 0, uv_count = 0;
    int v = 0, u = 0, f = 0;
    wv_model model = {0};
    FILE *file = fopen(filename, "r");

    if (file == NULL)
    {
        printf("Error opening %s\n", filename);
        return model;
    }

    while (fgets(line, sizeof(line), file))
    {
        // look for material definitions
        sscanf(line, "mtllib %s", mat_filename);

        // count vertices
        if (line[0] == 'v' && line[1] == ' ') vertex_count++;

        // count uvs
        if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') uv_count++;

        // count faces
        if (line[0] == 'f' && line[1] == ' ') face_count++;
    }

    // start over so we can read the actual data
    rewind(file);

    vector3 *vertices = malloc(sizeof(vector3) * vertex_count);
    vector2 *uvs = malloc(sizeof(vector2) * uv_count);
    wv_face *faces = malloc(sizeof(wv_face) * face_count);

    int mat_count = 0;
    char *curr_material_name;
    wv_material *materials = load_mat_file(mat_filename, &mat_count);

    while (fgets(line, sizeof(line), file))
    {
        char material_name[64];
        if (sscanf(line, "usemtl %s\n", material_name))
        {
            curr_material_name = malloc(sizeof(material_name));
            strcpy(curr_material_name, material_name);
        }

        // vertex data
        if (line[0] == 'v' && line[1] == ' ')
        {
            vector3 vertex = {0};
            sscanf(line, "v %f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            vertices[v] = vertex;
            v++;
        }

        // uv data
        if (line[0] == 'v' && line[1] == 't' && line[2] == ' ')
        {
            vector2 uv = {0};
            sscanf(line, "vt %f %f\n", &uv.x, &uv.y);
            uvs[u] = uv;
            u++;
        }

        // face data
        if (line[0] == 'f' && line[1] == ' ')
        {
            wv_face face;
            int ignore, matches, mat_idx;
            for (mat_idx = 0; mat_idx < mat_count; mat_idx++)
            {
                if (strcmp(materials[mat_idx].name, curr_material_name) == 0)
                {
                    face.material = &materials[mat_idx];
                    break;
                }
            }

            matches = sscanf(line, "f %i/%i %i/%i %i/%i\n",
                             &face.indices[0], &face.uvs[0],
                             &face.indices[1], &face.uvs[1],
                             &face.indices[2], &face.uvs[2]);

            if (matches == 6) faces[f] = face;

            matches = sscanf(line, "f %i/%i/%i %i/%i/%i %i/%i/%i\n",
                             &face.indices[0], &face.uvs[0], &ignore,
                             &face.indices[1], &face.uvs[1], &ignore,
                             &face.indices[2], &face.uvs[2], &ignore);

            if (matches == 9) faces[f] = face;

            f++;
        }
    }

    fclose(file);

    model.vertices = vertices;
    model.faces = faces;
    model.uvs = uvs;
    model.face_count = face_count;

    return model;
}
