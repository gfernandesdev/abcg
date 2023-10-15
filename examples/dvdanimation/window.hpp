#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <random>

#include "abcgOpenGL.hpp"

class Window : public abcg::OpenGLWindow {
protected:
  void onCreate() override;
  void onPaint() override;
  void onPaintUI() override;
  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;

private:
  glm::ivec2 m_viewportSize{};

  GLuint m_VAO{};
  GLuint m_VBOPositions{};
  GLuint m_VBOColors{};
  GLuint m_program{};

  std::default_random_engine m_randomEngine;

  int m_speed{50};
  int m_scale{25};

  float m_position_x{0};
  float m_position_y{0};
  int m_angle{30};

  int m_sides_of_pol{4};

  glm::vec4 m_current_color{1.0f, 1.0f, 1.0f, 1.0f};

  void setupModel(int sides);

  enum directions { UP, DOWN, LEFT, RIGHT };
  directions m_directionX{RIGHT};
  directions m_directionY{UP};

  enum axis { X, Y, NONE };
  axis m_collidedAxis{NONE};

  // funcoes auxiliares
  void setRandomColor();
  void movePolygon(float speed);
  void handleColision(axis collidedAxis);
};

#endif