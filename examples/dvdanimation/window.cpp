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

  // Gerando um angulo aleatorio entre 30 e 45
  std::uniform_int_distribution<int> rd_angle(0, 90);
  m_angle = rd_angle(m_randomEngine);

  // Gerando direcoes aleatorias
  std::uniform_int_distribution<int> rd_direction(0, 1);
  m_directionX = rd_direction(m_randomEngine) == 0 ? LEFT : RIGHT;
  m_directionY = rd_direction(m_randomEngine) == 0 ? DOWN : UP;

  fmt::print("Angulo inicial: {}\n", m_angle);

  // Cor inicial
  setRandomColor();
} 

void Window::onPaint() {
  // Definindo o poligono (default: quadrado)
  auto const sides{m_sides_of_pol};
  setupModel(sides);

  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  abcg::glUseProgram(m_program);

  // Definindo velocidade que ele se move com base no slider definido pelo user
  const float speed = 1e-6 * m_speed;

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
    handleColision(X);
  }

  if (m_position_y + (scale / 2.0) >= 1.0 ||
      m_position_y - (scale / 2.0) <= -1.0) {
    handleColision(Y);
  }

  // Movendo poligono com base na speed e direcao
  movePolygon(speed);

  std::uniform_real_distribution<float> rd_color(0.0f, 1.0f);

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
    static auto firstTime{true};
    if (firstTime) {
      ImGui::SetNextWindowPos(ImVec2(5, 75));
      firstTime = false;
    }

    ImGui::Begin("Settings of animation");

    ImGui::PushItemWidth(150);
    ImGui::SliderInt("Speed", &m_speed, 0, 5000, "%d");
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(150);
    ImGui::SliderInt("Scale", &m_scale, 10, 50, "%d");
    ImGui::PopItemWidth();

    // ImGui::PushItemWidth(150);
    // ImGui::SliderInt("Angle", &m_angle, 0, 360, "%d deg");
    // ImGui::PopItemWidth();

    // // Vertical Sliders para alterar cor
    // ImVec4 bgRed = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    // ImVec4 bgGreen = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    // ImVec4 bgBlue = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

    if (ImGui::Button("Angulo -90", ImVec2(-1, 30))) {
      m_angle = -90;
    }

    if (ImGui::Button("Angulo 45", ImVec2(-1, 30))) {
      m_angle = 45;
    }

    if (ImGui::Button("Angulo 90", ImVec2(-1, 30))) {
      m_angle = 90;
    }
    // Limpando quadrado atual para renderizar um novo
    abcg::glClear(GL_COLOR_BUFFER_BIT);

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

  // Minimum number of sides is 3
  sides = std::max(3, sides);

  std::vector<glm::vec2> positions;
  std::vector<glm::vec3> colors;

  // Polygon center
  positions.emplace_back(0, 0);
  colors.push_back(m_current_color);

  // Border vertices
  auto const step{M_PI * 2 / sides};
  for (auto const angle : iter::range(0.0, M_PI * 2, step)) {
    positions.emplace_back(std::cos(angle), std::sin(angle));
    colors.push_back(m_current_color);
  }

  // Duplicate second vertex
  positions.push_back(positions.at(1));
  colors.push_back(m_current_color);

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

void Window::setRandomColor() {
  // Gera uma color randomica
  std::uniform_real_distribution<float> rd_color(0.0f, 1.0f);
  m_current_color = {rd_color(m_randomEngine), rd_color(m_randomEngine),
                     rd_color(m_randomEngine), 1};
}

void Window::movePolygon(float speed) {
  // Funcao utilizada para mover o poligono
  float nextPositionX = speed * std::cos(m_angle * M_PI / 180);
  float nextPositionY = speed * std::sin(m_angle * M_PI / 180);

  m_position_x = (m_directionX == RIGHT) ? m_position_x + nextPositionX
                                         : m_position_x - nextPositionX;

  m_position_y = (m_directionY == UP) ? m_position_y + nextPositionY
                                      : m_position_y - nextPositionY;
}

void Window::handleColision(axis collidedAxis) {
  // Apos colisao X e Y mudam de direcao
  m_directionX = (m_directionX == RIGHT) ? LEFT : RIGHT;
  m_directionY = (m_directionY == UP) ? DOWN : UP;

  // Cor do poligono alterada randomicamente
  setRandomColor();

  static int tmp_angle{m_angle};

  // Define novo angulo com base no eixo que colidiu
  if (collidedAxis == X) {
    tmp_angle = 360.0 - tmp_angle;
  } else if (collidedAxis == Y) {
    tmp_angle = 180.0 - tmp_angle;
  }

  m_angle = normalizeAngle(tmp_angle);

  fmt::print("Colisao no eixo {}, novo angulo: {}\n",
             collidedAxis == X ? "X" : "Y", m_angle);
}

int Window::normalizeAngle(int angle) {
  // normalizacao para angulo ficar entre 0 e 360
  angle = std::fmod(angle, 360);

  if (angle < 0) {
    angle += 360;
  }

  return angle;
}