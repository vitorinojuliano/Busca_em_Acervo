#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <cctype>
#include <clocale>
#include <codecvt>
#include <locale>

using namespace std;

// --------------------------- Livro ----------------------------
struct Livro {
    int codigo;
    string titulo;
    string autor;
    string editora;
    int ano;
    int exemplares;
    string classificacao;
    string campus;

    Livro(int c, const string& t, const string& a, const string& e, int y, int ex, const string& cl, const string& cp)
        : codigo(c), titulo(t), autor(a), editora(e), ano(y), exemplares(ex), classificacao(cl), campus(cp) {}

    void print() const {
        cout << "Cod: " << codigo << endl;
        cout << "Tit: " << titulo << endl;
        cout << "Aut: " << (autor.empty() ? "Desconhecido" : autor) << endl;
        cout << "Edi: " << (editora.empty() ? "Desconhecida" : editora) << endl;
        cout << "Ano: " << (ano == 0 ? "Desconhecido" : to_string(ano)) << endl;
        cout << "Exem: " << exemplares << endl;
        cout << "Cla: " << classificacao << endl;
        cout << "Cam: " << campus << endl << endl;
    }
};

// --------------------- Nó da B+Tree --------------------------
const int ORDEM = 3;

struct BPlusNode {
    bool folha;
    vector<string> chaves;
    vector<BPlusNode*> filhos; // Se !folha
    vector<vector<Livro*>> valores; // Se folha: ponteiros para livros
    BPlusNode* proxFolha;

    BPlusNode(bool isFolha) : folha(isFolha), proxFolha(nullptr) {}
};

// ---------------------- B+Tree completa ------------------------
class BPlusTree {
private:
    BPlusNode* raiz;

    vector<string> dividirChaves(vector<string>& v, int pos) {
        vector<string> res(v.begin() + pos, v.end());
        v.resize(pos);
        return res;
    }

    vector<vector<Livro*>> dividirValores(vector<vector<Livro*>>& v, int pos) {
        vector<vector<Livro*>> res(v.begin() + pos, v.end());
        v.resize(pos);
        return res;
    }

    vector<BPlusNode*> dividirFilhos(vector<BPlusNode*>& v, int pos) {
        vector<BPlusNode*> res(v.begin() + pos, v.end());
        v.resize(pos);
        return res;
    }

    void inserirInterno(BPlusNode* node, const string& chave, Livro* livro) {
        if (node->folha) {
            auto it = lower_bound(node->chaves.begin(), node->chaves.end(), chave);
            int idx = distance(node->chaves.begin(), it);
            if (it != node->chaves.end() && *it == chave) {
                node->valores[idx].push_back(livro);
            } else {
                node->chaves.insert(it, chave);
                node->valores.insert(node->valores.begin() + idx, {livro});
            }
        } else {
            auto it = upper_bound(node->chaves.begin(), node->chaves.end(), chave);
            int idx = distance(node->chaves.begin(), it);
            inserirInterno(node->filhos[idx], chave, livro);

            if (node->filhos[idx]->chaves.size() >= ORDEM) {
                dividirNo(node, idx);
            }
        }
    }

    void dividirNo(BPlusNode* pai, int idx) {
        BPlusNode* filho = pai->filhos[idx];
        BPlusNode* novo = new BPlusNode(filho->folha);

        int mid = ORDEM / 2;
        string chavePromovida = filho->chaves[mid];

        novo->chaves = dividirChaves(filho->chaves, filho->folha ? mid : mid + 1);

        if (filho->folha) {
            novo->valores = dividirValores(filho->valores, mid);
            novo->proxFolha = filho->proxFolha;
            filho->proxFolha = novo;
        } else {
            novo->filhos = dividirFilhos(filho->filhos, mid + 1);
        }

        pai->chaves.insert(pai->chaves.begin() + idx, chavePromovida);
        pai->filhos.insert(pai->filhos.begin() + idx + 1, novo);
    }

public:
    BPlusTree() {
        raiz = new BPlusNode(true);
    }

    void inserir(const string& chave, Livro* livro) {
        inserirInterno(raiz, chave, livro);

        if (raiz->chaves.size() >= ORDEM) {
            BPlusNode* novaRaiz = new BPlusNode(false);
            novaRaiz->filhos.push_back(raiz);
            dividirNo(novaRaiz, 0);
            raiz = novaRaiz;
        }
    }

    vector<Livro*> buscarPrefixo(const string& prefixo) {
        vector<Livro*> resultado;
        BPlusNode* node = raiz;

        while (!node->folha) {
            auto it = upper_bound(node->chaves.begin(), node->chaves.end(), prefixo);
            int idx = distance(node->chaves.begin(), it);
            node = node->filhos[idx];
        }

        while (node) {
            for (size_t i = 0; i < node->chaves.size(); ++i) {
                if (node->chaves[i].compare(0, prefixo.size(), prefixo) == 0) {
                    resultado.insert(resultado.end(), node->valores[i].begin(), node->valores[i].end());
                } else if (node->chaves[i] > prefixo) {
                    return resultado;
                }
            }
            node = node->proxFolha;
        }
        return resultado;
    }
};

// ---------------------- Utilitários ------------------------
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

vector<string> extrairPalavras(const string& texto) {
    vector<string> palavras;
    stringstream ss(texto);
    string p;
    while (ss >> p) {
        transform(p.begin(), p.end(), p.begin(), [](unsigned char c){ return tolower(c); });
        palavras.push_back(p);
    }
    return palavras;
}

vector<string> parseCSVLine(const string& line) {
    vector<string> fields;
    bool inQuotes = false;
    string currentField;

    for (char c : line) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(trim(currentField));
            currentField.clear();
        } else {
            currentField += c;
        }
    }
    fields.push_back(trim(currentField));
    return fields;
}

int extrairAno(const string& data) {
    size_t pos = data.find('-');
    if (pos != string::npos) {
        string anoStr = data.substr(0, pos);
        try {
            return stoi(anoStr);
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

string extrairAutor(const string& entradaPrincipal) {
    if (entradaPrincipal.empty() || entradaPrincipal == " ") return "";
    
    size_t start = entradaPrincipal.find_first_not_of(" ");
    if (start == string::npos) return "";
    
    size_t end = entradaPrincipal.find_first_of(",;", start);
    if (end == string::npos) {
        end = entradaPrincipal.length();
    }
    
    return entradaPrincipal.substr(start, end - start);
}

// ------------------------ Main ----------------------------
int main(int argc, char* argv[]) {
    // Configurar locale para suportar caracteres acentuados
    setlocale(LC_ALL, "Portuguese_Brazil.1252"); // Configuração para Windows
    BPlusTree indiceTitulo;
    BPlusTree indiceAutor;
    unordered_map<int, Livro*> indiceCodigo;
    vector<Livro*> livros;

    // Verificar se o arquivo foi fornecido como parâmetro
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <arquivo_csv> [tipo_busca] [query]" << endl;
        return 1;
    }

    // Ler arquivo CSV
    ifstream arquivo(argv[1]);
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << argv[1] << endl;
        return 1;
    }

    string linha;
    getline(arquivo, linha); // Pular cabeçalho

    while (getline(arquivo, linha)) {
        vector<string> campos = parseCSVLine(linha);
        
        if (campos.size() < 25) continue; // Linha inválida

        int codigo = 0;
        try {
            codigo = stoi(campos[0]);
        } catch (...) {
            continue; // Ignorar linhas com código inválido
        }

        string titulo = campos[2];
        string autor = extrairAutor(campos[1]);
        string editora = campos[24];
        int ano = 0;
        try {
            ano = stoi(campos[11]); // Certifique-se de que o índice 11 corresponde ao campo "ano"
        } catch (...) {
            ano = 0; // Valor padrão para ano inválido
        }

        string classificacao = campos[10]; // Certifique-se de que o índice 10 corresponde ao campo "classificação"
        string campus = campos[20];        // Certifique-se de que o índice 20 corresponde ao campo "campus"

        int exemplares = 0;
        try {
            exemplares = stoi(campos[15]); // Certifique-se de que o índice 15 corresponde ao campo "exemplares"
        } catch (...) {
            exemplares = 1; // Valor padrão para exemplares inválidos
        }

        Livro* livro = new Livro(codigo, titulo, autor, editora, ano, exemplares, classificacao, campus);
        livros.push_back(livro);
        indiceCodigo[codigo] = livro;

        // Indexar por palavras do título
        for (const auto& palavra : extrairPalavras(titulo)) {
            indiceTitulo.inserir(palavra, livro);
        }

        // Indexar por autor (se existir)
        if (!autor.empty()) {
            for (const auto& palavra : extrairPalavras(autor)) {
                indiceAutor.inserir(palavra, livro);
            }
        }
    }

    arquivo.close();

    // Se parâmetros de busca foram fornecidos
    if (argc == 4) {
        string tipo_busca = argv[2];
        string query = argv[3];
        transform(query.begin(), query.end(), query.begin(), ::tolower);

        if (tipo_busca == "titulo") {
            vector<Livro*> resultado = indiceTitulo.buscarPrefixo(query);
            for (const auto* l : resultado) l->print();
        } else if (tipo_busca == "autor") {
            vector<Livro*> resultado = indiceAutor.buscarPrefixo(query);
            for (const auto* l : resultado) l->print();
        } else if (tipo_busca == "codigo") {
            int cod = stoi(query);
            if (indiceCodigo.count(cod)) indiceCodigo[cod]->print();
            else cout << "Livro não encontrado!" << endl;
        } else if (tipo_busca == "remove") {
            int cod = stoi(query);
            if (indiceCodigo.count(cod)) {
                Livro* livroRemovido = indiceCodigo[cod];
                cout << "Livro removido permanentemente!" << endl;
                livroRemovido->print();

                // Reescrever CSV sem o livro removido
                ifstream in(argv[1]);
                ofstream out("temp.csv");
                string linha;
                getline(in, linha); // cabeçalho
                out << linha << "\n";
                while (getline(in, linha)) {
                    vector<string> campos = parseCSVLine(linha);
                    if (campos.size() < 1) continue;
                    int codigoLinha = 0;
                    try { codigoLinha = stoi(campos[0]); } catch (...) {}
                    if (codigoLinha == cod) continue; // pula o livro removido
                    out << linha << "\n";
                }
                in.close();
                out.close();
                remove(argv[1]);
                rename("temp.csv", argv[1]);
            } else {
                cout << "Livro não encontrado!" << endl;
            }
        } else {
            cerr << "Tipo de busca inválido!" << endl;
            return 1;
        }
        return 0;
    } else if (argc >= 3) {
        string comando = argv[2];
        comando.erase(0, comando.find_first_not_of(" \t\n\r"));
        comando.erase(comando.find_last_not_of(" \t\n\r") + 1);
        transform(comando.begin(), comando.end(), comando.begin(), ::tolower);

        cout << "DEBUG: argc=" << argc << ", comando='" << comando << "', argv[3]='" << argv[3] << "'" << endl;

        if (comando == "add" && argc == 11) {
            cout << "DEBUG: Entrando no bloco ADD" << endl;
            // Monta a linha com 25 campos
            vector<string> campos(25, "");
            campos[0]  = argv[10]; // Código
            campos[2]  = argv[3];  // Título
            campos[1]  = argv[4];  // Autor
            campos[24] = argv[5];  // Editora
            campos[11] = argv[6];  // Ano
            campos[15] = argv[7];  // Exemplares
            campos[10] = argv[8];  // Classificação
            campos[20] = argv[9];  // Campus

            // campos[8] a campos[24] permanecem vazios

            // Gera a linha CSV
            string linha;
            for (size_t i = 0; i < campos.size(); ++i) {
                if (i > 0) linha += ",";
                linha += "\"" + campos[i] + "\"";
            }
            linha += "\n";

            ofstream out(argv[1], ios::app);
            out << linha;
            out.close();
            cout << "Livro adicionado com sucesso!" << endl;
            return 0;
        }
        else if (comando == "remove" && argc >= 4) {
            cout << "DEBUG: Entrando no bloco REMOVE" << endl;
            try {
                string codigoStr = argv[3];
                cout << "DEBUG: String do codigo recebida: '" << codigoStr << "'" << endl;
                int codigo = stoi(codigoStr);
                cout << "DEBUG: Codigo convertido para int: " << codigo << endl;
                cout << "DEBUG: Tamanho do mapa indiceCodigo: " << indiceCodigo.size() << endl;

                bool encontrado = false;
                Livro* livroRemovido = nullptr;

                if (indiceCodigo.count(codigo)) {
                    livroRemovido = indiceCodigo[codigo];
                    encontrado = true;
                    cout << "DEBUG: Livro encontrado! Título: " << livroRemovido->titulo << endl;
                    indiceCodigo.erase(codigo);
                    auto it = remove(livros.begin(), livros.end(), livroRemovido);
                    livros.erase(it, livros.end());
                    cout << "Livro removido temporariamente!" << endl;
                    livroRemovido->print();
                } else {
                    cout << "Livro nao encontrado!" << endl;
                }
            } catch (const std::exception& e) {
                cerr << "Erro ao converter codigo: " << e.what() << endl;
                return 1;
            }
            return 0;
        }
        // Se não for comando, tenta busca
        else if (argc == 4) {
            string tipo_busca = comando;
            string query = argv[3];
            transform(query.begin(), query.end(), query.begin(), ::tolower);

            if (tipo_busca == "titulo") {
                vector<Livro*> resultado = indiceTitulo.buscarPrefixo(query);
                for (const auto* l : resultado) l->print();
            } else if (tipo_busca == "autor") {
                vector<Livro*> resultado = indiceAutor.buscarPrefixo(query);
                for (const auto* l : resultado) l->print();
            } else if (tipo_busca == "codigo") {
                int cod = stoi(query);
                if (indiceCodigo.count(cod)) indiceCodigo[cod]->print();
                else cout << "Livro não encontrado!" << endl;
            } else {
                cerr << "Tipo de busca inválido!" << endl;
                return 1;
            }
            return 0;
        }
        else {
            cerr << "Tipo de busca inválido!" << endl;
            return 1;
        }
    }

    // Remova o menu interativo daqui!
    // Ele não deve ser executado quando há argumentos.
    // Se quiser manter para uso manual, coloque dentro de "if (argc < 2)".
    return 0;
}

// Compilação: g++ -o main.exe main.cpp
// Execução: ./main.exe ACERVO-DAS-BIBLIOTECAS-IFES-2025.csv
// python app.py
