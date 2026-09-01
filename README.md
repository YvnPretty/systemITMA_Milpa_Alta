# 🏫 Control Administrativo Escolar — Instituto Tecnológico de Milpa Alta II

![ITMA 2 Banner](itma2.webp)

Plataforma web moderna y profesional para el **Control Administrativo Escolar y Consulta de Listas de Alumnos** del Instituto Tecnológico de Milpa Alta II (ITMA 2). 

Diseñada con una interfaz interactiva basada en el sistema **iOS 18 Translucent Rosé Glassmorphism**, esta aplicación permite a docentes y coordinadores académicos cargar, consultar, filtrar, ordenar y visualizar expedientes en formato Excel (`.xlsx`, `.xls`, `.csv`) directamente desde el navegador de manera segura y sin requerir servidores externos.

🌐 **Despliegue en GitHub Pages:** [https://yvnpretty.github.io/systemITMA_Milpa_Alta/](https://yvnpretty.github.io/systemITMA_Milpa_Alta/)

---

## 🌟 Características Principales

- 🎨 **Estética iOS 18 Translucent Rosé Glassmorphism**:
  - Paneles traslúcidos rosados con efecto de desenfoque `backdrop-filter: blur(28px) saturate(190%)`.
  - Matrículas, IDs y códigos resaltados automáticamente en cápsulas de cristal traslúcido (*iOS Matrícula Pill*).
  - Identidad gráfica oficial del **Instituto Tecnológico de Milpa Alta II (`itma2.webp`)**.

- 📊 **Procesamiento Dinámico de Archivos Excel**:
  - Soporte completo para lectura de archivos `.xlsx`, `.xls` y `.csv` utilizando la librería `xlsx` (SheetJS).
  - Detección 100% automática de cabeceras y columnas arbitrarias.

- 🔍 **Consultas y Búsqueda en Tiempo Real**:
  - Buscador universal multi-campo sobre cualquier columna del archivo cargado.
  - Ordenamiento interactivo por columna (ascendente y descendente).
  - Paginación dinámica ajustable (10, 25, 50, 100 registros por página).

- 💾 **Persistencia Local Automática**:
  - Guarda automáticamente los datos cargados en el `localStorage` del navegador para conservar la lista sin necesidad de volver a subir el archivo al recargar la página.

---

## 🛠️ Tecnologías Utilizadas

- **Frontend Core**: JavaScript (ES6+), HTML5 Semántico.
- **Estilos**: Vanilla CSS3 (Custom Properties, Glassmorphism, Responsive Grid & Flexbox).
- **Librería de Excel**: `XLSX` (SheetJS) para lectura y parseo de hojas de cálculo.
- **Empaquetador y Compilador**: Webpack 5, Babel (`@babel/core`, `@babel/preset-env`).
- **Iconografía y Fuentes**: FontAwesome 6, Google Fonts (*Outfit*, *Plus Jakarta Sans*, *JetBrains Mono*).

---

## 🚀 Instalación y Ejecución Local

### Prerrequisitos
- Node.js (v16 o superior)
- NPM

### Pasos

1. **Clonar el repositorio:**
   ```bash
   git clone https://github.com/YvnPretty/systemITMA_Milpa_Alta.git
   cd systemITMA_Milpa_Alta
   ```

2. **Instalar dependencias:**
   ```bash
   npm install
   ```

3. **Iniciar el servidor de desarrollo:**
   ```bash
   npm start
   ```
   La aplicación estará disponible en `http://localhost:8085/`.

4. **Compilar para producción:**
   ```bash
   npm run build
   ```

---

## 🌐 Despliegue en GitHub Pages

Para habilitar la aplicación en GitHub Pages:
1. Ve a la sección **Settings** > **Pages** de tu repositorio en GitHub.
2. En **Build and deployment / Source**, selecciona la rama `main` y la carpeta `/ (root)`.
3. Haz clic en **Save**. En un par de minutos la aplicación estará desplegada públicamente.

---

## 📄 Licencia

Este proyecto está bajo la Licencia **MIT**. Desarrollado para el **Instituto Tecnológico de Milpa Alta II**.
