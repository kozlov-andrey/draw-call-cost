#pragma once

#ifdef _WIN32
#include <glad/glad.h>
#else
#include <GLES2/gl2.h>
#endif

#include <vector>

struct Color final
{
	Color() = default;
	Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

	float r = 1.f;
	float g = 0.f;
	float b = 0.f;
	float a = 1.0f;
};

struct Matrix final
{
	Matrix()
	{
		memset(values, 0, sizeof(values));
		values[0] = 1.0f;
		values[5] = 1.0f;
		values[10] = 1.0f;
		values[15] = 1.0f;
	}
	void SetTranslation(float x, float y, float z)
	{
		values[12] = x;
		values[13] = y;
		values[14] = z;
	}
	float values[16];
};

struct State final
{
	GLuint program = 0;
	GLuint color_location = 0;
	GLuint matrix_location = 0;
	Color color;
	Matrix martrix;
	bool alpha_blending_enabled = false;
	bool depth_write_enabled = false;
	bool depth_test_enabled = false;
};

struct DrawCall final
{
	State state;
	GLuint vbo = 0;
	GLuint ibo = 0;
	unsigned index_count = 0;
	unsigned offset_ = 0;
};

enum class ChangeStatePolicy
{
	DontChange,
	ChangeOnlyProgram,
	DontChangeProgram,
	Change,
};

class GLRenderer final
{
public:
	GLRenderer(int width, int height);

	void Setup(unsigned tri_per_batch, unsigned tri_per_frame, ChangeStatePolicy policy);
	void Draw();

private:
	int width_ = 0;
	int height_ = 0;
	GLuint color_location_[2] = { 0,0 };
	GLuint matrix_location_[2] = { 0,0 };
	GLuint program_object_[2] = {0,0};
	GLuint vbo_[2] = {0,0};
	GLuint ibo_[2] = {0,0};
	size_t index_count_ = 0;
	std::vector<DrawCall> draw_calls_;
	ChangeStatePolicy change_state_policy_;
};