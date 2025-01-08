#include <GL/glut.h>
#include <string>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <csignal>  // Para manipulação de sinais

// Dimensões da janela
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Estrutura para armazenar informações de processos
struct ProcessInfo {
    std::string name;
    int pid;
    float cpuUsage;
};

// Lista de processos (atualizada continuamente em background)
std::vector<ProcessInfo> processList;

// Variáveis para interação
bool isMouseOverKill = false, isMouseOverStop = false, isMouseOverCont = false;
bool isKillPressed = false, isStopPressed = false, isContPressed = false;
std::string pidInput = "";
std::string pidPriority = "";
float inputBoxX = -0.8f, inputBoxY = -0.6f, inputBoxWidth = 0.4f, inputBoxHeight = 0.1f;
// Ajuste nas posições das caixas de entrada
float inputBoxX2 = 0.2f, inputBoxY2 = -0.6f, inputBoxWidth2 = 0.4f, inputBoxHeight2 = 0.1f; // Nova posição para a segunda caixa

// Função para capturar processos reais usando o comando ps
void fetchProcessList() {
    while (true) {
        processList.clear();

        // Executa o comando ps para listar processos
        FILE* fp = popen("ps -eo pid,comm,%cpu --sort=-%cpu | head -n 10", "r");
        if (!fp) {
            std::cerr << "Erro ao executar comando ps!" << std::endl;
            return;
        }

        char buffer[1024];
        fgets(buffer, sizeof(buffer), fp); // Ignora a linha de cabeçalho

        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            ProcessInfo process;
            std::istringstream line(buffer);

            // Captura os valores: PID, Nome do Processo e %CPU
            line >> process.pid >> process.name >> process.cpuUsage;
            processList.push_back(process);
        }

        pclose(fp);

        // Atualiza a cada 1 segundo
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Função para desenhar texto na tela
void renderText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

// Função para desenhar a tabela
void renderProcessTable() {
    float startX = -0.9f;
    float startY = 0.8f;
    float rowHeight = 0.08f;
    float colWidthPID = 0.2f;   // Largura da coluna PID
    float colWidthName = 0.5f; // Largura da coluna Nome do Processo
    float colWidthCPU = 0.2f;  // Largura da coluna %CPU

    // Desenha os cabeçalhos da tabela
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(startX, startY);
    glVertex2f(startX + 1.0f, startY);
    glVertex2f(startX + 1.0f, startY - rowHeight);
    glVertex2f(startX, startY - rowHeight);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    renderText(startX + 0.05f, startY - 0.05f, "PID");
    renderText(startX + colWidthPID + 0.05f, startY - 0.05f, "Nome do Processo");
    renderText(startX + colWidthPID + colWidthName + 0.05f, startY - 0.05f, "% CPU");

    // Desenha linhas horizontais e verticais para a tabela
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.0f);

    for (size_t i = 0; i <= processList.size(); ++i) {
        float y = startY - (i + 1) * rowHeight;

        // Linha horizontal
        glBegin(GL_LINES);
        glVertex2f(startX, y);
        glVertex2f(startX + 1.0f, y);
        glEnd();
    }

    // Linhas verticais (divisórias das colunas)
    glBegin(GL_LINES);
    glVertex2f(startX + colWidthPID, startY);
    glVertex2f(startX + colWidthPID, startY - (processList.size() + 1) * rowHeight);

    glVertex2f(startX + colWidthPID + colWidthName, startY);
    glVertex2f(startX + colWidthPID + colWidthName, startY - (processList.size() + 1) * rowHeight);
    glEnd();

    // Preenche os dados dos processos
    for (size_t i = 0; i < processList.size(); ++i) {
        float y = startY - (i + 1) * rowHeight - 0.05f;

        std::ostringstream pidStream, cpuStream;
        pidStream << processList[i].pid;
        cpuStream << processList[i].cpuUsage;

        renderText(startX + 0.05f, y, pidStream.str());
        renderText(startX + colWidthPID + 0.05f, y, processList[i].name);
        renderText(startX + colWidthPID + colWidthName + 0.05f, y, cpuStream.str() + "%");
    }
}

// Função para desenhar botões na interface
void drawButton(float x, float y, float width, float height, const std::string& label, bool isHovered, bool isPressed) {
    if (isPressed)
        glColor3f(0.3f, 0.3f, 0.8f); // Azul escuro
    else if (isHovered)
        glColor3f(0.5f, 0.5f, 0.9f); // Azul claro
    else
        glColor3f(0.7f, 0.7f, 0.7f); // Cinza

    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y - height);
    glVertex2f(x, y - height);
    glEnd();

    // Desenha as bordas
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y - height);
    glVertex2f(x, y - height);
    glEnd();

    // Desenha o texto
    glColor3f(0.0f, 0.0f, 0.0f);
    renderText(x + width / 4.0f, y - height / 1.5f, label);
}


// Função para desenhar a caixa de entrada
void drawInputBox() {
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    // Primeira caixa
    glVertex2f(inputBoxX, inputBoxY);
    glVertex2f(inputBoxX + inputBoxWidth, inputBoxY);
    glVertex2f(inputBoxX + inputBoxWidth, inputBoxY - inputBoxHeight);
    glVertex2f(inputBoxX, inputBoxY - inputBoxHeight);
    glEnd();
    
    glBegin(GL_QUADS);
    // Segunda caixa
    glVertex2f(inputBoxX2, inputBoxY2);
    glVertex2f(inputBoxX2 + inputBoxWidth2, inputBoxY2);
    glVertex2f(inputBoxX2 + inputBoxWidth2, inputBoxY2 - inputBoxHeight2);
    glVertex2f(inputBoxX2, inputBoxY2 - inputBoxHeight2);
    glEnd();

    // Bordas
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(inputBoxX, inputBoxY);
    glVertex2f(inputBoxX + inputBoxWidth, inputBoxY);
    glVertex2f(inputBoxX + inputBoxWidth, inputBoxY - inputBoxHeight);
    glVertex2f(inputBoxX, inputBoxY - inputBoxHeight);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    glVertex2f(inputBoxX2, inputBoxY2);
    glVertex2f(inputBoxX2 + inputBoxWidth2, inputBoxY2);
    glVertex2f(inputBoxX2 + inputBoxWidth2, inputBoxY2 - inputBoxHeight2);
    glVertex2f(inputBoxX2, inputBoxY2 - inputBoxHeight2);
    glEnd();

    // Texto digitado
    renderText(inputBoxX + 0.05f, inputBoxY - 0.07f, pidInput); // Primeiro texto
    renderText(inputBoxX2 + 0.05f, inputBoxY2 - 0.07f, pidPriority); // Segundo texto
}


// Função para enviar sinal para matar, parar ou continuar um processo
void handleProcessAction(const std::string& action) {
    if (pidInput.empty()) return;

    int pid = std::stoi(pidInput);  // Converte o PID digitado para inteiro

    if (action == "Kill") {
        kill(pid, SIGKILL);  // Envia SIGKILL para matar o processo
    } else if (action == "Stop") {
        kill(pid, SIGSTOP);  // Envia SIGSTOP para parar o processo
    } else if (action == "Cont") {
        kill(pid, SIGCONT);  // Envia SIGCONT para continuar o processo
    }
}

// Função de display
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Renderiza a tabela de processos
    renderProcessTable();

    // Desenha os botões
    drawButton(-0.8f, -0.8f, 0.2f, 0.1f, "Kill", isMouseOverKill, isKillPressed);
    drawButton(-0.5f, -0.8f, 0.2f, 0.1f, "Stop", isMouseOverStop, isStopPressed);
    drawButton(-0.2f, -0.8f, 0.2f, 0.1f, "Cont", isMouseOverCont, isContPressed);

    // Desenha a caixa de entrada
    drawInputBox();

    glFlush();
    glutPostRedisplay();
}

// Função para lidar com eventos do mouse
void mouseMotion(int x, int y) {
    float mx = (float)x / (WINDOW_WIDTH / 2.0f) - 1.0f;
    float my = 1.0f - (float)y / (WINDOW_HEIGHT / 2.0f);

    isMouseOverKill = (mx >= -0.8f && mx <= -0.6f && my <= -0.8f && my >= -0.9f);
    isMouseOverStop = (mx >= -0.5f && mx <= -0.3f && my <= -0.8f && my >= -0.9f);
    isMouseOverCont = (mx >= -0.2f && mx <= 0.0f && my <= -0.8f && my >= -0.9f);
}

void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            if (isMouseOverKill) {
                handleProcessAction("Kill");
                isKillPressed = true;
            } else if (isMouseOverStop) {
                handleProcessAction("Stop");
                isStopPressed = true;
            } else if (isMouseOverCont) {
                handleProcessAction("Cont");
                isContPressed = true;
            }
        } else {
            isKillPressed = false;
            isStopPressed = false;
            isContPressed = false;
        }
    }
}

// Função para capturar teclas digitadas
void keyboard(unsigned char key, int x, int y) {
    if (key == 8 || key == 127) { // Backspace
        if (!pidInput.empty()) {
            pidInput.pop_back();
        }
    } else if (key >= 32 && key <= 126) { // Caracteres imprimíveis
        pidInput += key;
    }
}

// Função de inicialização do OpenGL
void init() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Fundo branco
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

// Função principal
int main(int argc, char** argv) {
    // Inicia o thread de atualização de processos
    std::thread processUpdater(fetchProcessList);
    processUpdater.detach();

    // Configuração do OpenGL
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Gerenciador de Tarefas");
    init();
    glutDisplayFunc(display);
    glutPassiveMotionFunc(mouseMotion);
    glutMouseFunc(mouseClick);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
