const sceneInput = document.getElementById('scene-input');
const renderButton = document.getElementById('render-btn');
const statusText = document.getElementById('status-text');
const statusSpinner = document.getElementById('status-spinner');
const canvas = document.getElementById('output-canvas');
const canvasContext = canvas.getContext('2d');

const defaultScenePath = '/tests/wasm_default_cornell_scene.txt';
const sceneFilePath = '/scene.txt';

let moduleInstance = null;

function waitForNextFrame() {
  return new Promise((resolve) => requestAnimationFrame(resolve));
}

function waitForPaintOpportunity() {
  return new Promise((resolve) => setTimeout(resolve, 0));
}

function setStatus(message, isRendering = false) {
  statusText.textContent = message;
  statusSpinner.classList.toggle('hidden', !isRendering);
  renderButton.disabled = isRendering;
}

async function loadDefaultScene() {
  try {
    const response = await fetch(defaultScenePath);
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }
    sceneInput.value = await response.text();
    setStatus('Scene loaded. Ready to render.');
  } catch (error) {
    setStatus(`Could not load default scene automatically: ${error.message}`);
  }
}

function pixelToRgba(pixel) {
  return [
    (pixel >>> 24) & 255,
    (pixel >>> 16) & 255,
    (pixel >>> 8) & 255,
    255,
  ];
}

function getHeapU32View() {
  if (moduleInstance.HEAPU32) {
    return moduleInstance.HEAPU32;
  }

  if (moduleInstance.HEAPU8) {
    return new Uint32Array(moduleInstance.HEAPU8.buffer);
  }

  if (moduleInstance.wasmMemory && moduleInstance.wasmMemory.buffer) {
    return new Uint32Array(moduleInstance.wasmMemory.buffer);
  }

  throw new Error('WASM memory view is unavailable. Rebuild with exported heap runtime methods.');
}

function presentImage() {
  const width = moduleInstance.ccall('get_width', 'number', [], []);
  const height = moduleInstance.ccall('get_height', 'number', [], []);
  const pixelsPtr = moduleInstance.ccall('get_pixels', 'number', [], []);

  if (!width || !height || !pixelsPtr) {
    throw new Error('Renderer did not return an image buffer.');
  }

  canvas.width = width;
  canvas.height = height;

  const pixelCount = width * height;
  const heapU32 = getHeapU32View();
  const pixels = heapU32.subarray(pixelsPtr >>> 2, (pixelsPtr >>> 2) + pixelCount);
  const imageData = canvasContext.createImageData(width, height);

  for (let y = 0; y < height; y += 1) {
    const srcY = height - 1 - y;
    for (let x = 0; x < width; x += 1) {
      const srcIndex = srcY * width + x;
      const [r, g, b, a] = pixelToRgba(pixels[srcIndex]);
      const offset = (y * width + x) * 4;
      imageData.data[offset] = r;
      imageData.data[offset + 1] = g;
      imageData.data[offset + 2] = b;
      imageData.data[offset + 3] = a;
    }
  }

  canvasContext.putImageData(imageData, 0, 0);
}

async function renderScene() {
  if (!moduleInstance) {
    return;
  }

  setStatus('Rendering...', true);

  // Yield to the browser so spinner/state updates can paint before wasm blocks.
  await waitForNextFrame();
  await waitForPaintOpportunity();

  const sceneText = sceneInput.value.endsWith('\n') ? sceneInput.value : `${sceneInput.value}\n`;
  moduleInstance.FS.writeFile(sceneFilePath, sceneText);
  moduleInstance.ccall('render_scene_file', null, ['string'], [sceneFilePath]);
  presentImage();

  setStatus('Render complete.');
}

renderButton.addEventListener('click', () => {
  renderScene().catch((error) => {
    console.error(error);
    setStatus(`Render failed: ${error.message}`);
  });
});

loadDefaultScene();

RayTracerModule({
  locateFile: (path) => `/wasm/${path}`,
}).then((instance) => {
  moduleInstance = instance;
  setStatus('WASM module loaded. Click Render.');
  renderScene().catch((error) => {
    console.error(error);
    setStatus(`Initial render failed: ${error.message}`);
  });
}).catch((error) => {
  console.error(error);
  setStatus(`Failed to load wasm module: ${error.message}`);
});