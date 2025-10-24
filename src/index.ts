// SPDX-License-Identifier: GPL-3.0-or-later

import W86, { type MainModule, type W86CpuState } from "./w86.js"

function updateBaseAddress(a: number): void {
  if (a < 0x00000) a = 0x00000;
  if (a > 0xfff00) a = 0xfff00;
  a -= a % 16;

  (<HTMLInputElement> emulator.ui.elements.namedItem("base-address")).value = a.toString(16).toUpperCase().padStart(5, "0");

  const rows: NodeList = document.getElementById("memory-view")?.querySelectorAll("tbody tr")!;
  for (let i: number = 0; i < 16; i++) {
    const row: Element = <Element> rows.item(i);
    row.querySelector("th")!.innerHTML = (a + i * 0x10).toString(16).toUpperCase().padStart(5, "0");
  }

  emulator.baseAddress = a;
}

function updateMemoryView(): void {
  const rows: NodeList = document.getElementById("memory-view")?.querySelectorAll("tbody tr")!;
  for (let i: number = 0; i < 16; i++) {
    const cells: NodeList = (<Element> rows.item(i)).querySelectorAll("td input");
    for (let j: number = 0; j < 16; j++) {
      const cell: HTMLInputElement = <HTMLInputElement> cells.item(j);
      cell.value = w86.HEAPU8[emulator.state.memory + emulator.baseAddress + i * 16 + j].toString(16).toUpperCase().padStart(2, "0");
    }
  }
}

function updateDisplay(): void {
  (<HTMLInputElement> emulator.ui.elements.namedItem("ax")).value = emulator.state.registers.ax.toString(16).toUpperCase().padStart(4, "0");
  (<HTMLInputElement> emulator.ui.elements.namedItem("bx")).value = emulator.state.registers.bx.toString(16).toUpperCase().padStart(4, "0");
  (<HTMLInputElement> emulator.ui.elements.namedItem("cx")).value = emulator.state.registers.cx.toString(16).toUpperCase().padStart(4, "0");
  (<HTMLInputElement> emulator.ui.elements.namedItem("dx")).value = emulator.state.registers.dx.toString(16).toUpperCase().padStart(4, "0");
  (<HTMLInputElement> emulator.ui.elements.namedItem("si")).value = emulator.state.registers.si.toString(16).toUpperCase().padStart(4, "0");
  (<HTMLInputElement> emulator.ui.elements.namedItem("di")).value = emulator.state.registers.di.toString(16).toUpperCase().padStart(4, "0");
  (<HTMLInputElement> emulator.ui.elements.namedItem("sp")).value = emulator.state.registers.sp.toString(16).toUpperCase().padStart(4, "0");
  (<HTMLInputElement> emulator.ui.elements.namedItem("bp")).value = emulator.state.registers.bp.toString(16).toUpperCase().padStart(4, "0");
  (<HTMLInputElement> emulator.ui.elements.namedItem("cs")).value = emulator.state.registers.cs.toString(16).toUpperCase().padStart(4, "0");
  (<HTMLInputElement> emulator.ui.elements.namedItem("ds")).value = emulator.state.registers.ds.toString(16).toUpperCase().padStart(4, "0");
  (<HTMLInputElement> emulator.ui.elements.namedItem("es")).value = emulator.state.registers.es.toString(16).toUpperCase().padStart(4, "0");
  (<HTMLInputElement> emulator.ui.elements.namedItem("ss")).value = emulator.state.registers.ss.toString(16).toUpperCase().padStart(4, "0");
  (<HTMLInputElement> emulator.ui.elements.namedItem("ip")).value = emulator.state.registers.ip.toString(16).toUpperCase().padStart(4, "0");
  for (let i: number = 0; i < 16; i++)
    (<HTMLInputElement> emulator.ui.elements.namedItem("flags" + i.toString())).checked = (emulator.state.registers.flags >> i & 1) !== 0;
  updateMemoryView();
}

function stepEmulator(): void {
  switch (w86.w86CpuStep(emulator.state)) {
  case w86.W86Status.SUCCESS:
    break;
  case w86.W86Status.UNDEFINED_OPCODE:
    console.error(`Undefined opcode: 0x${w86.HEAPU8[emulator.state.memory + ((emulator.state.registers.cs << 4) + emulator.state.registers.ip) % (1 << 20)].toString(16).toUpperCase().padStart(2, "0")}`);
    break;
  case w86.W86Status.UNIMPLEMENTED_OPCODE:
    console.error(`Unimplemented opcode: 0x${w86.HEAPU8[emulator.state.memory + ((emulator.state.registers.cs << 4) + emulator.state.registers.ip) % (1 << 20)].toString(16).toUpperCase().padStart(2, "0")}`);
    break;
  default:
    console.error("Unknown error");
  }
}

function resetEmulator(): void {
  emulator.state.registers = {
    ax: 0x0000,
    bx: 0x0000,
    cx: 0x0000,
    dx: 0x0000,
    si: 0x0000,
    di: 0x0000,
    sp: 0x0000,
    bp: 0x0000,
    cs: 0xffff,
    ds: 0x0000,
    es: 0x0000,
    ss: 0x0000,
    ip: 0x0000,
    flags: 0x0000
  };
  w86.HEAPU8.subarray(emulator.state.memory, emulator.state.memory + emulator.memorySize).set(new Uint8Array(emulator.rom));
}

{
  const rows: NodeList = document.getElementById("memory-view")?.querySelectorAll("tbody tr")!;
  const byte: HTMLInputElement = document.createElement("input");
  byte.type = "text";
  byte.disabled = true;
  byte.autocomplete = "off";
  byte.required = true;
  byte.size = 2;
  byte.maxLength = 2;
  byte.pattern = "[\dA-Fa-f]*";
  byte.placeholder = "00";
  byte.value = "00";
  const cell: Node = document.createElement("td");
  cell.appendChild(byte);
  for (const row of rows) {
    for (let i: number = 0; i < 16; i++) {
      row.appendChild(cell.cloneNode(true));
    }
  }
}

const w86: MainModule = await W86();

const emulator: {
  readonly state: W86CpuState;
  readonly memorySize: number;
  readonly rom: ArrayBuffer;
  baseAddress: number;
  readonly ui: HTMLFormElement;
} = {
  state: new w86.W86CpuState(),
  memorySize: 1048576,
  rom: new ArrayBuffer(1048576),
  baseAddress: 0x00000,
  ui: <HTMLFormElement> document.getElementById("emulator")
};
emulator.state.registers = {
  ax: 0x0000,
  bx: 0x0000,
  cx: 0x0000,
  dx: 0x0000,
  si: 0x0000,
  di: 0x0000,
  sp: 0x0000,
  bp: 0x0000,
  cs: 0xffff,
  ds: 0x0000,
  es: 0x0000,
  ss: 0x0000,
  ip: 0x0000,
  flags: 0x0000
};
emulator.state.memory = w86._malloc(emulator.memorySize);
w86.HEAPU8.subarray(emulator.state.memory, emulator.state.memory + emulator.memorySize).fill(0);

(<Element> emulator.ui.elements.namedItem("step")).addEventListener("click", (): void => {
  stepEmulator();
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("reset")).addEventListener("click", (): void => {
  resetEmulator();
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("rom")).addEventListener("change", (event: Event): void => {
  (<HTMLInputElement> event.currentTarget).files?.item(0)?.arrayBuffer().then((buf: ArrayBuffer): void => {
    const view: Uint8Array = new Uint8Array(emulator.rom);
    view.fill(0).set(new Uint8Array(buf).subarray(0, emulator.memorySize));
    resetEmulator();
    updateDisplay();
  });
});

(<Element> emulator.ui.elements.namedItem("base-address")).addEventListener("change", (event: Event): void => {
  const e: HTMLInputElement = <HTMLInputElement> event.currentTarget;
  let a: number;
  if (!e.checkValidity()) {
    a = 0x00000;
  } else {
    a = parseInt(e.value, 16);
  }
  updateBaseAddress(a);
  updateMemoryView();
});

(<Element> emulator.ui.elements.namedItem("first-address")).addEventListener("click", (): void => {
  updateBaseAddress(0x00000);
  updateMemoryView();
});

(<Element> emulator.ui.elements.namedItem("prev-address")).addEventListener("click", (): void => {
  updateBaseAddress(emulator.baseAddress - 0x100);
  updateMemoryView();
});

(<Element> emulator.ui.elements.namedItem("next-address")).addEventListener("click", (): void => {
  updateBaseAddress(emulator.baseAddress + 0x100);
  updateMemoryView();
});

(<Element> emulator.ui.elements.namedItem("last-address")).addEventListener("click", (): void => {
  updateBaseAddress(0xfff00);
  updateMemoryView();
});
