import requests
from flask import Flask, send_from_directory, jsonify

app = Flask(__name__)

ESP32_IP = '192.168.2.109'  # Replace with your ESP32's IP address

@app.route('/')
def index():
    return send_from_directory('.', 'index.html')

@app.route('/<path:filename>')
def serve_static(filename):
    return send_from_directory('.', filename)

@app.route('/esp32/depth')
def get_esp32_depth():
    try:
        response = requests.get(f'http://{ESP32_IP}/depth', timeout=5)
        response.raise_for_status()
        return jsonify(response.json())
    except requests.RequestException as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)
