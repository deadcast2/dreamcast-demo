#ifndef _MODEL_H_
#define _MODEL_H_

typedef struct
{
    float x;
    float y;
} vector2;

typedef struct
{
    float x;
    float y;
    float z;
} vector3;

typedef struct
{
    char *name;
    uint texture_id;
    float diffuse[3];
} wv_material;

typedef struct
{
    int indices[3];
    int uvs[3];
    wv_material *material;
} wv_face;

typedef struct
{
    vector3 *vertices;
    vector2 *uvs;
    wv_face *faces;
    int face_count;
} wv_model;

wv_model load(char *filename);

#endif