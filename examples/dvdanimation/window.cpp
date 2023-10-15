#include "window.hpp"

void Window::onCreate() {
  auto const *vertexShader{R"gl(#version 300 es

      layout(location = 0) in vec2 inPosition;
      layout(location = 1) in vec4 inColor;

      uniform vec2 translation;
      uniform float scale;
      uniform float rotation;

      out vec4 fragColor;

      void main() {
        float c = cos(rotation);
        float s = sin(rotation);
        mat2 rotationMatrix = mat2(c, -s, s, c);
        vec2 newPosition = rotationMatrix * inPosition * scale + translation;
        gl_Position = vec4(newPosition, 0, 1);
        fragColor = inColor;
      }
    )gl"};

  auto const *fragmentShader{R"gl(#version 300 es

    precision mediump float;  

    in vec4 fragColor;

    out vec4 outColor;

    void main() { outColor = fragColor; }
  )gl"};

  m_program = abcg::createOpenGLProgram(
      {{.source = vertexShader, .stage = abcg::ShaderStage::Vertex},
       {.source = fragmentShader, .stage = abcg::ShaderStage::Fragment}});

  abcg::glClearColor(0, 0, 0, 1);
  abcg::glClear(GL_COLOR_BUFFER_BIT);

  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());
}

void Window::onPaint() {
  if (m_timer.elapsed() < m_delay / 1000.0) {
    return;
  }
  m_timer.restart();

  // Definindo o poligono (default: quadrado)
  auto const sides{m_sides_of_pol};
  setupModel(sides);

  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  abcg::glUseProgram(m_program);

  // Definindo velocidade que ele se move com base no slider definido pelo user
  auto const speed{0.1f * (m_delay * 10)};
  // usando m_position_x como posicao inicial de X (default: 0)
  glm::vec2 const translation{m_position_x, m_position_y};
  auto const translationLocation{
      abcg::glGetUniformLocation(m_program, "translation")};
  abcg::glUniform2fv(translationLocation, 1, &translation.x);

  // Angulos
  // colisao inferior/superior, novo angulo = 360 - m_angle
  // colisao esquerda/direita, novo angulo = 180 - m_angle

  // Definindo tamanho do poligono, por padrao 25%
  auto const scale{m_scale / 100.0f};
  auto const scaleLocation{abcg::glGetUniformLocation(m_program, "scale")};
  abcg::glUniform1f(scaleLocation, scale);

  // definindo mudancas de direcao
  if (m_position_x + (scale / 2.0) >= 1.0 ||
      m_position_x - (scale / 2.0) <= -1.0) {

    m_direction_x = -1.0 * m_direction_x;
    m_direction_y = -1.0 * m_direction_y;
    

    m_angle = 360.0 - m_angle;

    fmt::print("m_position_x {}  m_angle {}\n", m_position_x, m_angle);
  }

  if (m_position_y + (scale / 2.0) >= 1.0 ||
      m_position_y - (scale / 2.0) <= -1.0) {
        
    m_direction_x = -1.0 * m_direction_x;
    m_direction_y = -1.0 * m_direction_y;

    m_angle = 180.0 - m_angle;

    fmt::print("m_position_y {} m_position_x {}  m_angle {}\n", m_position_y,
               m_position_x, m_angle);
  }

  // Incrementando ou decrementando X, com base em m_direction_x (direcao que
  // esta se movendo)
  if (m_direction_x > 0) {
    m_position_x += (speed / 10000) * std::cos(m_angle * M_PI / 180);
  } else {
    m_position_x -= (speed / 10000) * std::cos(m_angle * M_PI / 180);
  }

  if (m_direction_y > 0) {
    m_position_y += (speed / 10000) * std::sin(m_angle * M_PI / 180);
  } else {
    m_position_y -= (speed / 10000) * std::sin(m_angle * M_PI / 180);
  }

  // Definindo rotacao dele para quadrado ficar "correto"
  auto const rotation{M_PI / 4.0f};
  auto const rotationLocation{
      abcg::glGetUniformLocation(m_program, "rotation")};
  abcg::glUniform1f(rotationLocation, rotation);

  // Render
  abcg::glBindVertexArray(m_VAO);
  abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, sides + 2);
  abcg::glBindVertexArray(0);

  abcg::glUseProgram(0);
}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();

  {
    auto const widgetSize{ImVec2(200, 170)};
    ImGui::SetNextWindowPos(ImVec2(m_viewportSize.x - widgetSize.x - 5,
                                   m_viewportSize.y - widgetSize.y - 5));
    ImGui::SetNextWindowSize(widgetSize);
    auto const windowFlags{ImGuiWindowFlags_NoResize |
                           ImGuiWindowFlags_NoCollapse |
                           ImGuiWindowFlags_NoTitleBar};
    ImGui::Begin(" ", nullptr, windowFlags);

    ImGui::PushItemWidth(140);
    ImGui::SliderInt("Speed", &m_delay, 0, 200, "%d ms");
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(140);
    ImGui::SliderInt("Scale", &m_scale, 10, 50, "%d ms");
    ImGui::PopItemWidth();

    if (ImGui::Button("Circle", ImVec2(-1, 30))) {
      m_sides_of_pol = 100;
    }

    if (ImGui::Button("Square", ImVec2(-1, 30))) {
      m_sides_of_pol = 4;
    }

    if (ImGui::Button("Triangle", ImVec2(-1, 30))) {
      m_sides_of_pol = 3;
    }
    // Limpando quadrado atual para renderizar um novo
    if (!(m_timer.elapsed() < m_delay / 1000.0)) {
      abcg::glClear(GL_COLOR_BUFFER_BIT);
    }

    ImGui::End();
  }
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;

  abcg::glClear(GL_COLOR_BUFFER_BIT);
}

void Window::onDestroy() {
  abcg::glDeleteProgram(m_program);
  abcg::glDeleteBuffers(1, &m_VBOPositions);
  abcg::glDeleteBuffers(1, &m_VBOColors);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}

void Window::setupModel(int sides) {
  // Release previous resources, if any
  abcg::glDeleteBuffers(1, &m_VBOPositions);
  abcg::glDeleteBuffers(1, &m_VBOColors);
  abcg::glDeleteVertexArrays(1, &m_VAO);

  // Select random colors for the radial gradient
  glm::vec3 const color1{0.0f, 0.0f, 1.0f};
  glm::vec3 const color2{0.0f, 0.0f, 1.0f};

  // Minimum number of sides is 3
  sides = std::max(3, sides);

  std::vector<glm::vec2> positions;
  std::vector<glm::vec3> colors;

  // Polygon center
  positions.emplace_back(0, 0);
  colors.push_back(color1);

  // Border vertices
  auto const step{M_PI * 2 / sides};
  for (auto const angle : iter::range(0.0, M_PI * 2, step)) {
    positions.emplace_back(std::cos(angle), std::sin(angle));
    colors.push_back(color2);
  }

  // Duplicate second vertex
  positions.push_back(positions.at(1));
  colors.push_back(color2);

  // Generate VBO of positions
  abcg::glGenBuffers(1, &m_VBOPositions);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOPositions);
  abcg::glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                     positions.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate VBO of colors
  abcg::glGenBuffers(1, &m_VBOColors);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOColors);
  abcg::glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3),
                     colors.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  auto const positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};
  auto const colorAttribute{abcg::glGetAttribLocation(m_program, "inColor")};

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_VAO);

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOPositions);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glEnableVertexAttribArray(colorAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOColors);
  abcg::glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);
}