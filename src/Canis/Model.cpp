#include "Model.hpp"
#include "IOManager.hpp"
#include "Debug.hpp"

#include <GL/glew.h>

namespace Canis
{
    Model LoadModel(std::string _path)
    {
        Model model;
        model.path = _path;

        if (LoadOBJ(model.path, model.positions, model.uvs, model.normals) == false)
        {
            Canis::FatalError("Failed to load model at path " + model.path);
        }

        for (int i = 0; i < model.positions.size(); i++)
        {
            model.vertices.push_back(model.positions[i].x);
            model.vertices.push_back(model.positions[i].y);
            model.vertices.push_back(model.positions[i].z);
            model.vertices.push_back(model.normals[i].x);
            model.vertices.push_back(model.normals[i].y);
            model.vertices.push_back(model.normals[i].z);
            model.vertices.push_back(model.uvs[i].x);
            model.vertices.push_back(model.uvs[i].y);
        }

        glGenVertexArrays(1, &model.VAO);
        glGenBuffers(1, &model.VBO);

        glBindVertexArray(model.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, model.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.vertices.size(), &model.vertices[0], GL_STATIC_DRAW);

        // pos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // uv
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return model;
    }

    void Draw(Model &_model)
    {
        glBindVertexArray(_model.VAO);
        glDrawArrays(GL_TRIANGLES, 0, _model.vertices.size()/8);
        glBindVertexArray(0);
    }
}