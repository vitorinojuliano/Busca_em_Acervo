from flask import Flask, request, jsonify
import subprocess

app = Flask(__name__)

@app.route('/')
def home():
    return '''
    <h1>Sistema de Busca de Acervo</h1>
    <form action="/buscar" method="post">
        <label for="tipo">Tipo de busca:</label>
        <select name="tipo" id="tipo">
            <option value="titulo">Título</option>
            <option value="autor">Autor</option>
            <option value="codigo">Código</option>
        </select><br><br>
        <label for="query">Digite sua busca:</label>
        <input type="text" id="query" name="query"><br><br>
        <button type="submit">Buscar</button>
    </form>
    '''

@app.route('/buscar', methods=['POST'])
def buscar():
    tipo = request.form['tipo']
    query = request.form['query']

    try:
        # Executar o programa C++ com subprocess
        result = subprocess.run(
            ['.\\main.exe', 'ACERVO-DAS-BIBLIOTECAS-IFES-2025.csv', tipo, query],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        if result.returncode != 0:
            return f"<h1>Erro:</h1><pre>{result.stderr}</pre>"

        # Retornar o resultado para o navegador
        return f"<h1>Resultados:</h1><pre>{result.stdout}</pre>"
    except Exception as e:
        return f"<h1>Erro ao executar o programa:</h1><pre>{str(e)}</pre>"

if __name__ == '__main__':
    app.run(debug=True)