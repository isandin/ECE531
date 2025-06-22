from flask import Flask, request
app = Flask(__name__)

@app.route('/', methods=['GET', 'POST', 'PUT', 'DELETE'])
def handler():
    return f"{request.method} received: {request.data.decode()}", 200

app.run(host='0.0.0.0', port=8000)

