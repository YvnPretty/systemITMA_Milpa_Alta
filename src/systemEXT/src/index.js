import * as XLSX from 'xlsx';

// --- ESTADO GLOBAL Y CONFIGURACIÓN ---
const STORAGE_KEY = 'SCHOOL_ADMIN_EXCEL_DATA_V5';

let appState = {
  fileName: '',
  headers: [],
  records: [],
  filteredRecords: [],
  searchQuery: '',
  sortColumn: null,
  sortDirection: 'asc',
  currentPage: 1,
  pageSize: 25
};

// --- INICIALIZACIÓN ---
document.addEventListener('DOMContentLoaded', () => {
  // Limpiar cualquier residuo previo para inicio desde cero
  try {
    localStorage.removeItem('SCHOOL_ADMIN_EXCEL_DATA_V4');
    localStorage.removeItem('DYNAMIC_EXCEL_SYSTEM_DATA_V3');
    localStorage.removeItem('EXCEL_SYSTEM_DATABASE_V2');
  } catch(e) {}

  initApp();
});

function initApp() {
  loadStoredData();
  setupEventListeners();
  applyFilters();
}

// --- PERSISTENCIA LOCALSTORAGE ---
function loadStoredData() {
  try {
    const stored = localStorage.getItem(STORAGE_KEY);
    if (stored) {
      const parsed = JSON.parse(stored);
      if (parsed && Array.isArray(parsed.records) && parsed.records.length > 0) {
        appState.fileName = parsed.fileName || 'Lista_Escolar.xlsx';
        appState.headers = parsed.headers || Object.keys(parsed.records[0]);
        appState.records = parsed.records;
        showToast(`Lista cargada: "${appState.fileName}" (${appState.records.length} registros).`, 'info');
      }
    }
  } catch (e) {
    console.error('Error al cargar datos guardados:', e);
  }
}

function saveDataToStorage() {
  try {
    const payload = {
      fileName: appState.fileName,
      headers: appState.headers,
      records: appState.records
    };
    localStorage.setItem(STORAGE_KEY, JSON.stringify(payload));
  } catch (e) {
    console.error('Error guardando en memoria local:', e);
  }
}

// --- PARSING DE ARCHIVOS EXCEL ---
function handleFileUpload(file) {
  if (!file) return;

  const reader = new FileReader();
  reader.onload = (e) => {
    try {
      const data = new Uint8Array(e.target.result);
      const workbook = XLSX.read(data, { type: 'array' });

      if (!workbook.SheetNames || workbook.SheetNames.length === 0) {
        showToast('El archivo de Excel no contiene hojas de datos.', 'error');
        return;
      }

      const firstSheetName = workbook.SheetNames[0];
      const worksheet = workbook.Sheets[firstSheetName];
      
      const jsonRows = XLSX.utils.sheet_to_json(worksheet, { defval: '' });

      if (jsonRows.length === 0) {
        showToast('El archivo seleccionado está vacío.', 'error');
        return;
      }

      const headers = Object.keys(jsonRows[0]);

      appState.fileName = file.name;
      appState.headers = headers;
      appState.records = jsonRows;
      appState.sortColumn = null;
      appState.currentPage = 1;

      saveDataToStorage();
      applyFilters();

      showToast(`¡Lista "${file.name}" cargada correctamente! (${jsonRows.length} registros).`, 'success');
    } catch (err) {
      console.error('Error al leer Excel:', err);
      showToast('Error al leer el archivo Excel. Revisa el formato.', 'error');
    }
  };

  reader.readAsArrayBuffer(file);
}

// --- BÚSQUEDA Y FILTRADO EN TIEMPO REAL ---
function applyFilters() {
  let result = [...appState.records];

  // Búsqueda multi-campo en cualquier columna
  if (appState.searchQuery.trim() !== '') {
    const q = appState.searchQuery.toLowerCase().trim();
    result = result.filter(rec => {
      return Object.values(rec).some(val => {
        if (val === null || val === undefined) return false;
        return val.toString().toLowerCase().includes(q);
      });
    });
  }

  // Ordenamiento por columna
  if (appState.sortColumn) {
    const col = appState.sortColumn;
    const dir = appState.sortDirection === 'asc' ? 1 : -1;

    result.sort((a, b) => {
      let valA = a[col] ?? '';
      let valB = b[col] ?? '';

      const numA = Number(valA);
      const numB = Number(valB);

      if (!isNaN(numA) && !isNaN(numB) && valA !== '' && valB !== '') {
        return (numA - numB) * dir;
      }

      return valA.toString().localeCompare(valB.toString(), 'es', { numeric: true }) * dir;
    });
  }

  appState.filteredRecords = result;

  const maxPage = Math.ceil(result.length / appState.pageSize) || 1;
  if (appState.currentPage > maxPage) {
    appState.currentPage = maxPage;
  }

  renderUI();
}

// --- RENDERIZADO UI ---
function renderUI() {
  const hasData = appState.records.length > 0;

  const fileInfoBar = document.getElementById('file-info-bar');
  const querySection = document.getElementById('query-section');
  const pageSizeContainer = document.getElementById('page-size-container');
  const paginationSection = document.getElementById('pagination-section');

  if (fileInfoBar) fileInfoBar.style.display = hasData ? 'flex' : 'none';
  if (querySection) querySection.style.display = hasData ? 'flex' : 'none';
  if (pageSizeContainer) pageSizeContainer.style.display = hasData ? 'flex' : 'none';
  if (paginationSection) paginationSection.style.display = hasData ? 'flex' : 'none';

  if (hasData) {
    document.getElementById('loaded-file-name').textContent = appState.fileName;
    document.getElementById('loaded-file-meta').textContent = `${appState.records.length} registros cargados | ${appState.headers.length} columnas`;
  }

  renderTableHeaders();
  renderTableRows();
  renderPagination();
}

function renderTableHeaders() {
  const thead = document.getElementById('table-head');
  if (appState.headers.length === 0) {
    thead.innerHTML = '';
    return;
  }

  const thHTML = appState.headers.map(h => {
    const isSorted = appState.sortColumn === h;
    const sortIcon = isSorted 
      ? (appState.sortDirection === 'asc' ? '<i class="fa-solid fa-arrow-up-z-a" style="color:#e11d48;"></i>' : '<i class="fa-solid fa-arrow-down-z-a" style="color:#e11d48;"></i>') 
      : '<i class="fa-solid fa-sort" style="opacity: 0.35;"></i>';

    return `
      <th data-sort="${escapeHTML(h)}" title="Ordenar por ${escapeHTML(h)}">
        ${escapeHTML(h)} ${sortIcon}
      </th>
    `;
  }).join('');

  thead.innerHTML = `<tr>${thHTML}</tr>`;
}

function renderTableRows() {
  const tbody = document.getElementById('table-body');
  const badge = document.getElementById('filtered-count-badge');

  if (badge) {
    badge.textContent = `${appState.filteredRecords.length} resultados`;
  }

  // Estado inicial sin datos
  if (appState.records.length === 0) {
    tbody.innerHTML = `
      <tr>
        <td colspan="100">
          <div class="empty-state">
            <i class="fa-solid fa-folder-open empty-icon"></i>
            <h4 style="font-size: 1.15rem; font-weight: 700; color: #881337;">Esperando Lista o Archivo de Excel</h4>
            <p style="font-size: 0.88rem; color: #9f1239; margin-top: 6px; max-width: 480px; margin-left: auto; margin-right: auto;">
              Selecciona o arrastra tu archivo de Excel (.xlsx, .xls, .csv) en el recuadro superior para desplegar la información.
            </p>
          </div>
        </td>
      </tr>
    `;
    return;
  }

  // Sin resultados de búsqueda
  if (appState.filteredRecords.length === 0) {
    tbody.innerHTML = `
      <tr>
        <td colspan="100">
          <div class="empty-state">
            <i class="fa-solid fa-magnifying-glass" style="font-size: 38px; margin-bottom: 12px; color: #fb7185;"></i>
            <h4 style="font-size: 1.05rem; font-weight: 700; color: #881337;">No se encontraron registros con esa búsqueda</h4>
            <p style="font-size: 0.88rem; color: #9f1239; margin-top: 4px;">Intenta buscar por otro término o limpia el buscador.</p>
          </div>
        </td>
      </tr>
    `;
    return;
  }

  // Paginación slice
  const startIndex = (appState.currentPage - 1) * appState.pageSize;
  const pageRecords = appState.filteredRecords.slice(startIndex, startIndex + appState.pageSize);

  tbody.innerHTML = pageRecords.map(rec => {
    const cellsHTML = appState.headers.map(h => {
      const val = rec[h] ?? '';
      const valStr = val.toString();
      const hLower = h.toLowerCase();

      let cellContent = escapeHTML(valStr);
      const lower = valStr.toLowerCase();

      if (hLower.includes('matr') || hLower.includes('id') || hLower.includes('cod') || hLower.includes('cód')) {
        cellContent = `<span class="matricula-pill">${escapeHTML(valStr)}</span>`;
      } else if (lower === 'en curso' || lower === 'activo' || lower === 'inscrito') {
        cellContent = `<span class="status-pill status-en-curso"><i class="fa-solid fa-circle" style="font-size: 6px;"></i> ${escapeHTML(valStr)}</span>`;
      } else if (lower === 'finalizado' || lower === 'aprobado' || lower === 'acreditado') {
        cellContent = `<span class="status-pill status-finalizado"><i class="fa-solid fa-circle" style="font-size: 6px;"></i> ${escapeHTML(valStr)}</span>`;
      } else if (lower === 'pendiente') {
        cellContent = `<span class="status-pill status-pendiente"><i class="fa-solid fa-circle" style="font-size: 6px;"></i> ${escapeHTML(valStr)}</span>`;
      }

      return `<td>${cellContent}</td>`;
    }).join('');

    return `<tr>${cellsHTML}</tr>`;
  }).join('');
}

function renderPagination() {
  const total = appState.filteredRecords.length;
  const start = total === 0 ? 0 : (appState.currentPage - 1) * appState.pageSize + 1;
  const end = Math.min(appState.currentPage * appState.pageSize, total);
  const totalPages = Math.ceil(total / appState.pageSize) || 1;

  const infoEl = document.getElementById('pagination-info');
  const indEl = document.getElementById('page-indicator');

  if (infoEl) infoEl.textContent = `Mostrando ${start}-${end} de ${total} registros`;
  if (indEl) indEl.textContent = `Página ${appState.currentPage} de ${totalPages}`;

  const btnPrev = document.getElementById('btn-prev-page');
  const btnNext = document.getElementById('btn-next-page');

  if (btnPrev) btnPrev.disabled = appState.currentPage <= 1;
  if (btnNext) btnNext.disabled = appState.currentPage >= totalPages;
}

// --- EVENT LISTENERS ---
function setupEventListeners() {
  const searchInput = document.getElementById('search-input');
  if (searchInput) {
    searchInput.addEventListener('input', (e) => {
      appState.searchQuery = e.target.value;
      appState.currentPage = 1;
      applyFilters();
    });
  }

  document.getElementById('btn-clear-search')?.addEventListener('click', () => {
    appState.searchQuery = '';
    if (searchInput) searchInput.value = '';
    appState.currentPage = 1;
    applyFilters();
  });

  document.getElementById('btn-prev-page')?.addEventListener('click', () => {
    if (appState.currentPage > 1) {
      appState.currentPage--;
      renderTableRows();
      renderPagination();
    }
  });

  document.getElementById('btn-next-page')?.addEventListener('click', () => {
    const totalPages = Math.ceil(appState.filteredRecords.length / appState.pageSize);
    if (appState.currentPage < totalPages) {
      appState.currentPage++;
      renderTableRows();
      renderPagination();
    }
  });

  document.getElementById('page-size-select')?.addEventListener('change', (e) => {
    appState.pageSize = parseInt(e.target.value, 10);
    appState.currentPage = 1;
    applyFilters();
  });

  document.getElementById('table-head')?.addEventListener('click', (e) => {
    const th = e.target.closest('th[data-sort]');
    if (!th) return;

    const col = th.getAttribute('data-sort');
    if (appState.sortColumn === col) {
      appState.sortDirection = appState.sortDirection === 'asc' ? 'desc' : 'asc';
    } else {
      appState.sortColumn = col;
      appState.sortDirection = 'asc';
    }
    applyFilters();
  });

  document.getElementById('btn-change-file')?.addEventListener('click', () => {
    document.getElementById('excel-file-input')?.click();
  });

  const excelInput = document.getElementById('excel-file-input');
  const dropzone = document.getElementById('dropzone');

  if (excelInput) {
    excelInput.addEventListener('change', (e) => {
      if (e.target.files && e.target.files[0]) {
        handleFileUpload(e.target.files[0]);
      }
    });
  }

  if (dropzone) {
    ['dragenter', 'dragover'].forEach(eventName => {
      dropzone.addEventListener(eventName, (e) => {
        e.preventDefault();
        dropzone.classList.add('drag-over');
      }, false);
    });

    ['dragleave', 'drop'].forEach(eventName => {
      dropzone.addEventListener(eventName, (e) => {
        e.preventDefault();
        dropzone.classList.remove('drag-over');
      }, false);
    });

    dropzone.addEventListener('drop', (e) => {
      const dt = e.dataTransfer;
      if (dt.files && dt.files[0]) {
        handleFileUpload(dt.files[0]);
      }
    });
  }
}

// --- UTILIDADES ---
function showToast(message, type = 'info') {
  const container = document.getElementById('toast-container');
  if (!container) return;

  const toast = document.createElement('div');
  toast.className = `toast toast-${type}`;
  
  let iconClass = 'fa-circle-info';
  if (type === 'success') iconClass = 'fa-circle-check';
  if (type === 'error') iconClass = 'fa-triangle-exclamation';

  toast.innerHTML = `
    <i class="fa-solid ${iconClass}"></i>
    <span>${escapeHTML(message)}</span>
  `;

  container.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    toast.style.transform = 'translateX(100%)';
    toast.style.transition = 'all 0.3s ease';
    setTimeout(() => toast.remove(), 300);
  }, 4000);
}

function escapeHTML(str) {
  if (typeof str !== 'string') return str;
  return str.replace(/[&<>'"]/g, 
    tag => ({
      '&': '&amp;',
      '<': '&lt;',
      '>': '&gt;',
      "'": '&#39;',
      '"': '&quot;'
    }[tag] || tag)
  );
}
