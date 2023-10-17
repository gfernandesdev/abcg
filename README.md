# COMPUTAÇÃO GRÁFICA - Aplicação Interativa 2D

## Integrantes 

Nome: Gabriel Fernandes             RA: 11201720718
Nome: Matheus Alexandre de Sena     RA: 11201720166

## Link para WebAssembly

https://gfernandesdev.github.io/dvdanimation/dvdanimation/index.html


## Resumo

O projeto `dvdanimation` teve como inspiração o protetor de tela animado `DVD Bouncing`, muito utilizado nas décadas passadas em DVD Players, no qual o logotipo percorria batendo nos quatro cantos da tela alterando sua coloração a cada vez que tocava em um desses cantos.
Para este projeto, foi utilizado a biblioteca `ABCg` (https://github.com/hbatagelo/abcg) disponibilizada no curso de Computação Gráfica 2023.3 na Universidade Federal do ABC.  


## Funcionalidade

* window.hpp:

A classe Window é definida e herda da classe abcg::OpenGLWindow, que é uma parte da biblioteca abcgOpenGL para criar janelas gráficas.

Variáveis definidas:

`m_speed`: velocidade do polígono 
`m_scale`: tamanho do polígono
`m_position_x` e `m_position_y`: posição no plano
`m_angle`: ângulo a percorrer
`m_sides_of_pol`: número de lados do polígono
`m_current_color`: cor atual do polígono

Funções auxiliares declaradas:

`setRandomColor()`: defini uma cor randomicamente
`movePolygon()`: move o polígono
`handleColision()`: trata as colisões com os cantos da tela

* window.cpp:

O arquivo window.cpp é composto pelas funções utilizadas para a construção lógica da aplicação renderizada. Abaixo segue um resumo descritivo de cada função presente no arquivo:

`onCreate`: Foi configurado vertexShader e fragmentShader. Além disso, são configuradas variáveis iniciais, como a semente do gerador de números aleatórios, ângulo inicial, direções aleatórias e cor inicial.
`onPaint`: A geometria do polígono é definida, e a animação é renderizada. Isso inclui configurar as transformações de translação, rotação e escala, verificar e lidar com colisões nas bordas da tela e movimentar o polígono.
`onPaintUI`: Define uma interface de usuário (UI) usando a biblioteca ImGui. Ela inclui controles para ajustar a velocidade e escala da animação, e três botões para definir o ângulo do polígono (-90, 45, 90).
`onResize`: É chamado quando a janela é redimensionada e atualiza o tamanho do viewport.
`onDestroy`: É chamado quando a janela é destruída e é usado para limpar os recursos OpenGL.
`setupModel`: É usado para configurar a geometria do polígono, criando VAO e VBOs para as posições e cores dos vértices.
`setRandomColor`: Gera uma cor aleatória para o polígono.
`movePolygon`: É responsável por mover o polígono com base na velocidade e direção atual.
`handleColision`: Lida com colisões nas bordas da tela, atualizando as direções de movimento, a cor e o ângulo do polígono.

* main.cpp:

O arquivo main.cpp inicia a aplicação, criando uma instância da classe Window e realizando as configurações da janela de exibição (width, height, title). Em seguida, inicia a aplicação com `app.run(window)`.

