const char* HTML_UI = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Control de Temperatura</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background-color: #f4f4f4;
      margin: 0;
      padding: 0;
    }
    .button {
      display: inline-block;
      padding: 10px 20px;
      font-size: 16px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
      margin: 10px;
      width: 80%; /* Ancho ajustable */
      max-width: 200px; /* Máximo ancho */
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    #chartButton {
      background-color: #008CBA;
    }
    #chartButton:hover {
      background-color: #005f6a;
    }

    @media screen and (max-width: 600px) {
      .button {
        width: 90%;
        padding: 15px 20px;
        font-size: 18px;
      }
      h1 {
        font-size: 24px;
      }
    }
  </style>
</head>
<body>
  <h1>Control de Temperatura</h1>
  <button class="button" id="onButton">ON</button>
  <button class="button" id="offButton">OFF</button>
  <br>
  <button class="button" id="chartButton">Mostrar Gráfica</button>

  <script>
    document.getElementById('onButton').addEventListener('click', function() {
      fetch('/on') // Hace una petición GET a la ruta /on
        .then(response => response.text())
        .then(data => alert(data)); // Muestra una alerta con la respuesta
  });
  </script>
</body>
</html>
)rawliteral";