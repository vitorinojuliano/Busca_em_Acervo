from flask import Flask, request
import subprocess

app = Flask(__name__)

HTML_TEMPLATE_WITH_NAV = '''
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <title>ORI</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.10.5/font/bootstrap-icons.css">
</head>
<body class="bg-dark">
<nav class="navbar navbar-expand-lg navbar-dark bg-dark mb-4">
  <div class="container-fluid">
    <a class="navbar-brand" href="/">Sistema de Acervo</a>
    <div>
      <a class="btn btn-success me-2" href="/buscar">Buscar</a>
      <a class="btn btn-info me-2" href="/adicionar">Adicionar</a>
      <a class="btn btn-danger" href="/remover">Remover</a>
    </div>
  </div>
</nav>
<div class="container">
    {conteudo}
</div>
</body>
</html>
'''

HTML_TEMPLATE_NO_NAV = '''
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <title>Biblioteca IFES</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.10.5/font/bootstrap-icons.css">
</head>
<body class="bg-dark">
<div class="container">
    {conteudo}
</div>
</body>
</html>
'''

@app.route('/')
def home():
    return HTML_TEMPLATE_NO_NAV.format(conteudo='''
    <div class="d-flex flex-column align-items-center justify-content-center" style="min-height:70vh;">
        <h2 class="mb-5 fw-bold text-light">Sistema de Acervo</h2>
        <div class="row g-5">
            <div class="col text-center">
                <a href="/buscar" class="btn btn-lg btn-success shadow px-5 py-4">
                    <span class="fs-2 mb-2 d-block"><i class="bi bi-search"></i></span>
                    <span class="fs-4">Buscar</span>
                </a>
            </div>
            <div class="col text-center">
                <a href="/adicionar" class="btn btn-lg btn-info shadow px-5 py-4">
                    <span class="fs-2 mb-2 d-block"><i class="bi bi-plus-circle"></i></span>
                    <span class="fs-4">Adicionar</span>
                </a>
            </div>
            <div class="col text-center">
                <a href="/remover" class="btn btn-lg btn-danger shadow px-5 py-4">
                    <span class="fs-2 mb-2 d-block"><i class="bi bi-trash"></i></span>
                    <span class="fs-4">Remover</span>
                </a>
            </div>
        </div>
    </div>
    ''')

@app.route('/buscar', methods=['GET', 'POST'])
def buscar():
    resultados = ""
    if request.method == 'POST':
        tipo = request.form['tipo']
        query = request.form['query']

        try:
            result = subprocess.run(
                ['.\\main.exe', 'ACERVO-DAS-BIBLIOTECAS-IFES-2025.csv', tipo, query],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )

            if result.returncode != 0:
                resultados = f'<div class="alert alert-danger mt-4"><pre>{result.stderr}</pre></div>'
            else:
                linhas = [l for l in result.stdout.strip().split('\n') if l.strip()]
                if linhas and linhas[0].startswith("Cod:"):
                    blocos = []
                    for i in range(0, len(linhas), 8):
                        campos = {}
                        for j, nome in enumerate(["Cod", "Tit", "Aut", "Edi", "Ano", "Exem", "Cla", "Cam"]):
                            if i + j < len(linhas) and ':' in linhas[i + j]:
                                k, v = linhas[i + j].split(':', 1)
                                campos[nome] = v.strip()
                        bloco = f'''
<div class="alert alert-info mt-4">
    <ul class="mb-0">
        <li><b>Código:</b> {campos.get("Cod", "")}</li>
        <li><b>Título:</b> {campos.get("Tit", "")}</li>
        <li><b>Autor:</b> {campos.get("Aut", "")}</li>
        <li><b>Editora:</b> {campos.get("Edi", "")}</li>
        <li><b>Ano:</b> {campos.get("Ano", "")}</li>
        <li><b>Exemplares:</b> {campos.get("Exem", "")}</li>
        <li><b>Classificação:</b> {campos.get("Cla", "")}</li>
        <li><b>Campus:</b> {campos.get("Cam", "")}</li>
    </ul>
</div>
'''
                        blocos.append(bloco)
                    resultados = "\n".join(blocos)
                else:
                    resultados = f'<div class="alert alert-info mt-4"><pre>{result.stdout}</pre></div>'

        except Exception as e:
            resultados = f'<div class="alert alert-danger mt-4"><pre>{str(e)}</pre></div>'

    conteudo = f'''
    <div class="card shadow mb-4">
        <div class="card-header bg-success text-white">
            <h4>Buscar Livro</h4>
        </div>
        <div class="card-body">
            <form action="/buscar" method="post" class="row g-3 mb-4">
                <div class="col-md-4">
                    <label for="tipo" class="form-label">Tipo de busca:</label>
                    <select name="tipo" id="tipo" class="form-select">
                        <option value="titulo">Título</option>
                        <option value="autor">Autor</option>
                        <option value="codigo">Código</option>
                    </select>
                </div>
                <div class="col-md-6">
                    <label for="query" class="form-label">Digite sua busca:</label>
                    <input type="text" id="query" name="query" class="form-control" required>
                </div>
                <div class="col-md-2 d-flex align-items-end">
                    <button type="submit" class="btn btn-success w-100">Buscar</button>
                </div>
            </form>
            {resultados}
            <a href="/" class="btn btn-secondary mt-3">&larr; Voltar</a>
        </div>
    </div>
    '''
    return HTML_TEMPLATE_WITH_NAV.format(conteudo=conteudo)

@app.route('/adicionar', methods=['GET', 'POST'])
def adicionar():
    msg = ""
    if request.method == 'POST':
        codigo        = request.form['codigo']
        titulo        = request.form['titulo']
        autor         = request.form['autor']
        editora       = request.form['editora']
        ano           = request.form['ano']
        exemplares    = request.form['exemplares']
        classificacao = request.form['classificacao']
        campus        = request.form['campus']

        cmd = [
            '.\\main.exe', 'ACERVO-DAS-BIBLIOTECAS-IFES-2025.csv',
            'add', titulo, autor, editora,
            ano, exemplares, classificacao, campus, codigo
        ]
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if result.returncode:
            msg = f'<div class="alert alert-danger"><pre>{result.stderr}</pre></div>'
        else:
            msg = f'''
<div class="alert alert-success mt-4">
    <strong>Livro adicionado com sucesso!</strong>
    <ul class="mb-0">
        <li><b>Código:</b> {codigo}</li>
        <li><b>Título:</b> {titulo}</li>
        <li><b>Autor:</b> {autor}</li>
        <li><b>Editora:</b> {editora}</li>
        <li><b>Ano:</b> {ano}</li>
        <li><b>Exemplares:</b> {exemplares}</li>
        <li><b>Classificação:</b> {classificacao}</li>
        <li><b>Campus:</b> {campus}</li>
    </ul>
</div>
'''
    conteudo = f'''
    <div class="card shadow mb-4">
        <div class="card-header bg-info text-white">
            <h4>Adicionar Livro</h4>
        </div>
        <div class="card-body">
            <form action="/adicionar" method="post" class="row g-3">
                <input name="codigo"      class="form-control" placeholder="Código" required>
                <input name="titulo"      class="form-control" placeholder="Título" required>
                <input name="autor"       class="form-control" placeholder="Autor">
                <input name="editora"     class="form-control" placeholder="Editora">
                <input name="ano"         class="form-control" placeholder="Ano">
                <input name="exemplares"  class="form-control" placeholder="Exemplares">
                <input name="classificacao" class="form-control" placeholder="Classificação">
                <input name="campus"      class="form-control" placeholder="Campus">
                <button type="submit" class="btn btn-primary mt-2">Adicionar</button>
            </form>
            {msg}
            <a href="/" class="btn btn-secondary mt-3">&larr; Voltar</a>
        </div>
    </div>
    '''
    return HTML_TEMPLATE_WITH_NAV.format(conteudo=conteudo)

@app.route('/remover', methods=['GET', 'POST'])
def remover():
    msg = ""
    if request.method == 'POST':
        codigo = request.form['codigo'].strip()  # <-- precisa existir no formulário!
        cmd = ['.\\main.exe', 'ACERVO-DAS-BIBLIOTECAS-IFES-2025.csv', 'remove', codigo]
        print(f"DEBUG FLASK: Comando completo: {cmd}")
        print(f"DEBUG FLASK: Código enviado: '{codigo}'")
        
        result = subprocess.run(
            cmd, 
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE, 
            text=True,
            encoding='utf-8',
            errors='replace'
        )
        
        print(f"DEBUG FLASK: Return code: {result.returncode}")
        print(f"DEBUG FLASK: STDOUT: {result.stdout}")
        print(f"DEBUG FLASK: STDERR: {result.stderr}")
        
        if result.returncode:
            msg = f'<div class="alert alert-danger"><pre>{result.stderr}</pre></div>'
        else:
            # Tenta formatar a saída do C++ (se ela imprime os campos como "Cod: ...", "Tit: ...", etc)
            linhas = [l for l in result.stdout.strip().split('\n') if l.strip()]
            if linhas and linhas[0].startswith("Livro removido"):
                # Pega os próximos campos
                campos = {}
                for l in linhas[1:]:
                    if ':' in l:
                        k, v = l.split(':', 1)
                        campos[k.strip()] = v.strip()
                msg = f'''
        <div class="alert alert-warning mt-4">
            <strong>Livro removido!</strong>
            <ul class="mb-0">
                <li><b>Código:</b> {campos.get("Cod", "")}</li>
                <li><b>Título:</b> {campos.get("Tit", "")}</li>
                <li><b>Autor:</b> {campos.get("Aut", "")}</li>
                <li><b>Editora:</b> {campos.get("Edi", "")}</li>
                <li><b>Ano:</b> {campos.get("Ano", "")}</li>
                <li><b>Exemplares:</b> {campos.get("Exem", "")}</li>
                <li><b>Classificação:</b> {campos.get("Cla", "")}</li>
                <li><b>Campus:</b> {campos.get("Cam", "")}</li>
            </ul>
        </div>
        '''
            else:
                msg = f'<div class="alert alert-warning"><pre>{result.stdout}</pre></div>'
    conteudo = f'''
    <div class="card shadow mb-4">
        <div class="card-header bg-danger text-white">
            <h4>Remover Livro</h4>
        </div>
        <div class="card-body">
            <form action="/remover" method="post" class="row g-3 align-items-end">
                <div class="col-md-9">
                    <input name="codigo" class="form-control" placeholder="Código do livro" required>
                </div>
                <div class="col-md-3">
                    <button type="submit" class="btn btn-danger w-100">Remover</button>
                </div>
            </form>
            {msg}
            <a href="/" class="btn btn-secondary mt-3">&larr; Voltar</a>
        </div>
    </div>
    '''
    return HTML_TEMPLATE_WITH_NAV.format(conteudo=conteudo)

@app.route('/menu')
def menu():
    return HTML_TEMPLATE_WITH_NAV.format(conteudo='''
    <div class="d-flex flex-column align-items-center justify-content-center" style="min-height:70vh;">
        <h2 class="mb-5 fw-bold text-light">Menu do Sistema de Busca de Acervo</h2>
        <div class="row g-5">
            <div class="col text-center">
                <a href="/buscar" class="btn btn-lg btn-success shadow px-5 py-4">
                    <span class="fs-2 mb-2 d-block"><i class="bi bi-search"></i></span>
                    <span class="fs-4">Buscar</span>
                </a>
            </div>
            <div class="col text-center">
                <a href="/adicionar" class="btn btn-lg btn-info shadow px-5 py-4">
                    <span class="fs-2 mb-2 d-block"><i class="bi bi-plus-circle"></i></span>
                    <span class="fs-4">Adicionar</span>
                </a>
            </div>
            <div class="col text-center">
                <a href="/remover" class="btn btn-lg btn-danger shadow px-5 py-4">
                    <span class="fs-2 mb-2 d-block"><i class="bi bi-trash"></i></span>
                    <span class="fs-4">Remover</span>
                </a>
            </div>
        </div>
    </div>
    ''')

if __name__ == '__main__':
    app.run(debug=True)