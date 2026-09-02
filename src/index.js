import * as XLSX from 'xlsx';

// --- ESTADO GLOBAL Y CONFIGURACIÓN ---
const STORAGE_KEY = 'SCHOOL_CREDITS_DATA_V8';

const STANDARD_HEADERS = [
  'No. de Control',
  'Nombre',
  'Carrera',
  'Crédito Cívico',
  'Crédito Cultural',
  'Crédito Deportivo',
  'Periodo',
  'Estatus'
];

let appState = {
  fileName: '',
  records: [],
  filteredRecords: [],
  searchQuery: '',
  statusFilter: 'ALL',
  sortColumn: 'matricula',
  sortDirection: 'asc',
  currentPage: 1,
  pageSize: 25
};

// --- INICIALIZACIÓN ---
document.addEventListener('DOMContentLoaded', () => {
  try {
    localStorage.removeItem('SCHOOL_CREDITS_DATA_V7');
    localStorage.removeItem('SCHOOL_CREDITS_DATA_V6');
    localStorage.removeItem('SCHOOL_ADMIN_EXCEL_DATA_V5');
  } catch (e) {}
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
        appState.fileName = parsed.fileName || 'Lista_Alumnos_Creditos.xlsx';
        appState.records = parsed.records;
        showToast(`Base de créditos cargada: "${appState.fileName}" (${appState.records.length} matriculados).`, 'info');
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
      records: appState.records
    };
    localStorage.setItem(STORAGE_KEY, JSON.stringify(payload));
  } catch (e) {
    console.error('Error guardando en memoria local:', e);
  }
}

// --- PARSING DE ARCHIVOS EXCEL E INTELIGENCIA DE COLUMNAS ---
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

      // Mapeo inteligente a los 8 campos estandarizados de créditos
      const normalizedRecords = jsonRows.map((row, idx) => {
        const getVal = (...keys) => {
          for (const k of keys) {
            const match = Object.keys(row).find(rk => rk.toLowerCase().trim() === k.toLowerCase().trim());
            if (match && row[match] !== undefined && row[match] !== '') {
              return row[match];
            }
          }
          return '';
        };

        const rawEstatus = getVal('Estatus', 'Estado', 'Liberación', 'Liberacion', 'Estatus General');
        let estatusNormalized = 'Pendiente';
        if (rawEstatus.toString().toLowerCase().includes('liber') || rawEstatus.toString().toLowerCase().includes('aprob') || rawEstatus.toString().toLowerCase().includes('final')) {
          estatusNormalized = 'Liberado';
        }

        return {
          id: `STD-${Date.now()}-${idx}`,
          matricula: getVal('No. de Control', 'Número de Control', 'Numero de Control', 'Matrícula', 'Matricula', 'ID', 'Control') || `1510${6000 + idx}`,
          nombre: getVal('Nombre', 'Nombre Completo', 'Alumno', 'Nombre del Alumno') || 'Alumno Registrado',
          carrera: getVal('Carrera', 'Especialidad') || 'INGENIERÍA EN SISTEMAS COMPUTACIONALES',
          civico: getVal('Crédito Cívico', 'Credito Civico', 'Cívico', 'Civico') || '',
          cultural: getVal('Crédito Cultural', 'Credito Cultural', 'Cultural') || '',
          deportivo: getVal('Crédito Deportivo', 'Credito Deportivo', 'Deportivo') || '',
          periodo: getVal('Periodo', 'Periodo de Cobertura', 'Semestre') || '',
          estatus: estatusNormalized
        };
      });

      appState.fileName = file.name;
      appState.records = normalizedRecords;
      appState.sortColumn = 'matricula';
      appState.currentPage = 1;

      saveDataToStorage();
      applyFilters();

      showToast(`¡Lista "${file.name}" cargada! ${normalizedRecords.length} alumnos listos para asignación de créditos.`, 'success');
    } catch (err) {
      console.error('Error al leer Excel:', err);
      showToast('Error al leer el archivo Excel. Revisa el formato.', 'error');
    }
  };

  reader.readAsArrayBuffer(file);
}

// --- BÚSQUEDA Y FILTRADO ---
function applyFilters() {
  let result = [...appState.records];

  // 1. Filtro por Estatus (Pendiente / Liberado)
  if (appState.statusFilter !== 'ALL') {
    result = result.filter(rec => rec.estatus.toLowerCase() === appState.statusFilter.toLowerCase());
  }

  // 2. Búsqueda multi-campo
  if (appState.searchQuery.trim() !== '') {
    const q = appState.searchQuery.toLowerCase().trim();
    result = result.filter(rec => {
      return (
        rec.matricula.toLowerCase().includes(q) ||
        rec.nombre.toLowerCase().includes(q) ||
        rec.carrera.toLowerCase().includes(q) ||
        rec.civico.toLowerCase().includes(q) ||
        rec.cultural.toLowerCase().includes(q) ||
        rec.deportivo.toLowerCase().includes(q) ||
        rec.periodo.toLowerCase().includes(q) ||
        rec.estatus.toLowerCase().includes(q)
      );
    });
  }

  // 3. Ordenamiento
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
    document.getElementById('loaded-file-meta').textContent = `${appState.records.length} matriculados cargados | 8 columnas de créditos`;
  }

  renderTableHeaders();
  renderTableRows();
  renderPagination();
}

function renderTableHeaders() {
  const thead = document.getElementById('table-head');
  
  const headersMap = [
    { label: 'No. de Control', key: 'matricula' },
    { label: 'Nombre Completo', key: 'nombre' },
    { label: 'Carrera', key: 'carrera' },
    { label: 'Crédito Cívico', key: 'civico' },
    { label: 'Crédito Cultural', key: 'cultural' },
    { label: 'Crédito Deportivo', key: 'deportivo' },
    { label: 'Periodo', key: 'periodo' },
    { label: 'Estatus', key: 'estatus' }
  ];

  const thHTML = headersMap.map(h => {
    const isSorted = appState.sortColumn === h.key;
    const sortIcon = isSorted 
      ? (appState.sortDirection === 'asc' ? '<i class="fa-solid fa-arrow-up-z-a" style="color:#e11d48;"></i>' : '<i class="fa-solid fa-arrow-down-z-a" style="color:#e11d48;"></i>') 
      : '<i class="fa-solid fa-sort" style="opacity: 0.35;"></i>';

    return `
      <th data-sort="${h.key}" title="Ordenar por ${h.label}">
        ${h.label} ${sortIcon}
      </th>
    `;
  }).join('') + '<th style="text-align: right;">Acciones</th>';

  thead.innerHTML = `<tr>${thHTML}</tr>`;
}

function renderTableRows() {
  const tbody = document.getElementById('table-body');
  const badge = document.getElementById('filtered-count-badge');

  if (badge) {
    badge.textContent = `${appState.filteredRecords.length} matriculados`;
  }

  // Estado inicial sin datos
  if (appState.records.length === 0) {
    tbody.innerHTML = `
      <tr>
        <td colspan="9">
          <div class="empty-state">
            <i class="fa-solid fa-file-circle-plus empty-icon"></i>
            <h4 style="font-size: 1.15rem; font-weight: 700; color: #881337;">Esperando Lista de Matriculados</h4>
            <p style="font-size: 0.88rem; color: #9f1239; margin-top: 6px; max-width: 500px; margin-left: auto; margin-right: auto;">
              Arrastra tu archivo de Excel (.xlsx, .xls, .csv) arriba o haz clic en <strong>"Nuevo Alumno"</strong> para gestionar sus créditos cívicos, culturales y deportivos.
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
        <td colspan="9">
          <div class="empty-state">
            <i class="fa-solid fa-magnifying-glass" style="font-size: 38px; margin-bottom: 12px; color: #fb7185;"></i>
            <h4 style="font-size: 1.05rem; font-weight: 700; color: #881337;">Sin coincidencias en el expediente</h4>
            <p style="font-size: 0.88rem; color: #9f1239; margin-top: 4px;">Intenta cambiar el filtro o escribe otra palabra clave.</p>
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
    const isLiberado = rec.estatus.toLowerCase() === 'liberado';
    const statusPill = isLiberado
      ? `<span class="status-pill status-finalizado"><i class="fa-solid fa-circle-check"></i> Liberado</span>`
      : `<span class="status-pill status-pendiente"><i class="fa-solid fa-clock"></i> Pendiente</span>`;

    return `
      <tr>
        <td><span class="matricula-pill">${escapeHTML(rec.matricula)}</span></td>
        <td style="font-weight: 700; color: #881337;">${escapeHTML(rec.nombre)}</td>
        <td style="font-size: 0.84rem; color: var(--text-muted);">${escapeHTML(rec.carrera)}</td>
        <td style="font-weight: 600;">${escapeHTML(rec.civico || '—')}</td>
        <td style="font-weight: 600;">${escapeHTML(rec.cultural || '—')}</td>
        <td style="font-weight: 600;">${escapeHTML(rec.deportivo || '—')}</td>
        <td style="font-size: 0.84rem; font-weight: 600; color: var(--accent-rose-dark);">${escapeHTML(rec.periodo || '—')}</td>
        <td>${statusPill}</td>
        <td style="text-align: right; white-space: nowrap;">
          <button class="btn btn-secondary btn-sm btn-edit-record" data-id="${escapeHTML(rec.id)}" title="Editar Créditos">
            <i class="fa-solid fa-pen-to-square" style="color: #e11d48;"></i> Editar
          </button>
          <button class="btn btn-secondary btn-sm btn-delete-record" data-id="${escapeHTML(rec.id)}" title="Eliminar" style="color: #be123c;">
            <i class="fa-solid fa-trash-can"></i>
          </button>
        </td>
      </tr>
    `;
  }).join('');
}

function renderPagination() {
  const total = appState.filteredRecords.length;
  const start = total === 0 ? 0 : (appState.currentPage - 1) * appState.pageSize + 1;
  const end = Math.min(appState.currentPage * appState.pageSize, total);
  const totalPages = Math.ceil(total / appState.pageSize) || 1;

  const infoEl = document.getElementById('pagination-info');
  const indEl = document.getElementById('page-indicator');

  if (infoEl) infoEl.textContent = `Mostrando ${start}-${end} de ${total} matriculados`;
  if (indEl) indEl.textContent = `Página ${appState.currentPage} de ${totalPages}`;

  const btnPrev = document.getElementById('btn-prev-page');
  const btnNext = document.getElementById('btn-next-page');

  if (btnPrev) btnPrev.disabled = appState.currentPage <= 1;
  if (btnNext) btnNext.disabled = appState.currentPage >= totalPages;
}

// --- MODAL Y EDICIÓN DE CRÉDITOS ---
function openRecordModal(id = null) {
  const modal = document.getElementById('modal-record');
  const title = document.getElementById('modal-record-title');
  const form = document.getElementById('record-form');
  form.reset();

  if (id) {
    title.innerHTML = '<i class="fa-solid fa-user-pen" style="color: #f472b6;"></i> Editar Créditos de Alumno';
    const rec = appState.records.find(r => r.id === id);
    if (rec) {
      document.getElementById('form-record-id').value = rec.id;
      document.getElementById('form-matricula').value = rec.matricula;
      document.getElementById('form-nombre').value = rec.nombre;
      document.getElementById('form-carrera').value = rec.carrera;
      document.getElementById('form-civico').value = rec.civico;
      document.getElementById('form-cultural').value = rec.cultural;
      document.getElementById('form-deportivo').value = rec.deportivo;
      document.getElementById('form-periodo').value = rec.periodo;
      document.getElementById('form-estatus').value = rec.estatus || 'Pendiente';
    }
  } else {
    title.innerHTML = '<i class="fa-solid fa-user-plus" style="color: #f472b6;"></i> Nuevo Alumno en Expediente';
    document.getElementById('form-record-id').value = '';
    document.getElementById('form-matricula').value = `1510${Math.floor(1000 + Math.random() * 9000)}`;
    document.getElementById('form-carrera').value = 'INGENIERÍA EN SISTEMAS COMPUTACIONALES';
  }

  modal.classList.add('active');
}

function closeRecordModal() {
  document.getElementById('modal-record')?.classList.remove('active');
}

function saveRecordFromForm() {
  const id = document.getElementById('form-record-id').value;
  const matricula = document.getElementById('form-matricula').value.trim();
  const nombre = document.getElementById('form-nombre').value.trim();
  const carrera = document.getElementById('form-carrera').value.trim();
  const civico = document.getElementById('form-civico').value.trim();
  const cultural = document.getElementById('form-cultural').value.trim();
  const deportivo = document.getElementById('form-deportivo').value.trim();
  const periodo = document.getElementById('form-periodo').value.trim();
  const estatus = document.getElementById('form-estatus').value;

  if (id) {
    const index = appState.records.findIndex(r => r.id === id);
    if (index !== -1) {
      appState.records[index] = {
        ...appState.records[index],
        matricula,
        nombre,
        carrera,
        civico,
        cultural,
        deportivo,
        periodo,
        estatus
      };
      showToast(`Créditos actualizados para "${nombre}".`, 'success');
    }
  } else {
    const newRecord = {
      id: `STD-${Date.now()}`,
      matricula,
      nombre,
      carrera,
      civico,
      cultural,
      deportivo,
      periodo,
      estatus
    };
    appState.records.unshift(newRecord);
    showToast(`Nuevo alumno "${nombre}" registrado en expediente.`, 'success');
  }

  saveDataToStorage();
  closeRecordModal();
  applyFilters();
}

function deleteRecord(id) {
  const rec = appState.records.find(r => r.id === id);
  const nombre = rec ? rec.nombre : 'este alumno';

  if (confirm(`¿Estás seguro de eliminar el registro de "${nombre}" del expediente?`)) {
    appState.records = appState.records.filter(r => r.id !== id);
    saveDataToStorage();
    applyFilters();
    showToast('Alumno eliminado del expediente.', 'info');
  }
}

// --- EXPORTAR A EXCEL ---
function exportToExcel() {
  if (appState.filteredRecords.length === 0) {
    showToast('No hay registros para guardar en Excel.', 'info');
    return;
  }

  try {
    const exportData = appState.filteredRecords.map(r => ({
      'No. de Control': r.matricula,
      'Nombre': r.nombre,
      'Carrera': r.carrera,
      'Crédito Cívico': r.civico || '',
      'Crédito Cultural': r.cultural || '',
      'Crédito Deportivo': r.deportivo || '',
      'Periodo': r.periodo || '',
      'Estatus': r.estatus
    }));

    const worksheet = XLSX.utils.json_to_sheet(exportData, { header: STANDARD_HEADERS });
    const workbook = XLSX.utils.book_new();
    XLSX.utils.book_append_sheet(workbook, worksheet, 'Creditos_Alumnos');

    const fileNameBase = appState.fileName ? appState.fileName.replace(/\.[^/.]+$/, '') : 'Registro_Creditos_ITMA2';
    XLSX.writeFile(workbook, `${fileNameBase}_Actualizado.xlsx`);
    showToast('Archivo Excel guardado y descargado exitosamente.', 'success');
  } catch (err) {
    console.error('Error al guardar Excel:', err);
    showToast('Error al generar el archivo Excel.', 'error');
  }
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

  const statusFilterSelect = document.getElementById('status-filter-select');
  if (statusFilterSelect) {
    statusFilterSelect.addEventListener('change', (e) => {
      appState.statusFilter = e.target.value;
      appState.currentPage = 1;
      applyFilters();
    });
  }

  document.getElementById('btn-clear-search')?.addEventListener('click', () => {
    appState.searchQuery = '';
    appState.statusFilter = 'ALL';
    if (searchInput) searchInput.value = '';
    if (statusFilterSelect) statusFilterSelect.value = 'ALL';
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

  document.getElementById('btn-add-record')?.addEventListener('click', () => openRecordModal(null));
  document.getElementById('btn-export-excel')?.addEventListener('click', exportToExcel);
  document.getElementById('btn-change-file')?.addEventListener('click', () => document.getElementById('excel-file-input')?.click());

  document.getElementById('table-body')?.addEventListener('click', (e) => {
    const btnEdit = e.target.closest('.btn-edit-record');
    const btnDelete = e.target.closest('.btn-delete-record');

    if (btnEdit) {
      const id = btnEdit.getAttribute('data-id');
      openRecordModal(id);
    } else if (btnDelete) {
      const id = btnDelete.getAttribute('data-id');
      deleteRecord(id);
    }
  });

  document.getElementById('record-form')?.addEventListener('submit', (e) => {
    e.preventDefault();
    saveRecordFromForm();
  });

  document.getElementById('modal-record-close')?.addEventListener('click', closeRecordModal);
  document.getElementById('modal-record-cancel')?.addEventListener('click', closeRecordModal);

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
