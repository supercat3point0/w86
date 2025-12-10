// SPDX-License-Identifier: GPL-3.0-or-later

import W86, { type MainModule, type W86CpuState } from "./w86.js"

interface Emulator {
  readonly state: W86CpuState;
  memory: Uint8Array;
  program: Uint8Array;
  io: {
    reads: Uint8Array;
    writes: Uint8Array;
  };
  execState: {
    run: boolean;
    halt: boolean;
    error?: string;
  };
  readonly memorySize: number;
  readonly ioSize: number;
  base: {
    memory: number;
    program: number;
    io: {
      reads: number;
      writes: number;
    };
  };
  readonly ui: HTMLFormElement;
}

function modifyRegister(event: Event): void {
  const e: HTMLInputElement = <HTMLInputElement> event.currentTarget;
  if (!e.checkValidity()) {
    updateDisplay();
    return;
  }

  // we have to do it like this because value_objects are immutable
  switch (e.name) {
  case "ax":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { ax: parseInt(e.value, 16) });
    break;

  case "bx":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { bx: parseInt(e.value, 16) });
    break;

  case "cx":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { cx: parseInt(e.value, 16) });
    break;

  case "dx":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { dx: parseInt(e.value, 16) });
    break;

  case "si":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { si: parseInt(e.value, 16) });
    break;

  case "di":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { di: parseInt(e.value, 16) });
    break;

  case "sp":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { sp: parseInt(e.value, 16) });
    break;

  case "bp":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { bp: parseInt(e.value, 16) });
    break;

  case "cs":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { cs: parseInt(e.value, 16) });
    break;

  case "ds":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { ds: parseInt(e.value, 16) });
    break;

  case "es":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { es: parseInt(e.value, 16) });
    break;

  case "ss":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { ss: parseInt(e.value, 16) });
    break;

  case "ip":
    emulator.state.registers = Object.assign({}, emulator.state.registers, { ip: parseInt(e.value, 16) });
  }

  updateDisplay();
}

function modifyFlags(event: Event): void {
  const e: HTMLInputElement = <HTMLInputElement> event.currentTarget;

  let shift;
  switch (e.name) {
  case "flags0":
    shift = 0;
    break;

  case "flags1":
    shift = 1;
    break;

  case "flags2":
    shift = 2;
    break;

  case "flags3":
    shift = 3;
    break;

  case "flags4":
    shift = 4;
    break;
    
  case "flags5":
    shift = 5;
    break;

  case "flags6":
    shift = 6;
    break;

  case "flags7":
    shift = 7;
    break;

  case "flags8":
    shift = 8;
    break;

  case "flags9":
    shift = 9;
    break;

  case "flags10":
    shift = 10;
    break;

  case "flags11":
    shift = 11;
    break;

  case "flags12":
    shift = 12;
    break;

  case "flags13":
    shift = 13;
    break;

  case "flags14":
    shift = 14;
    break;

  case "flags15":
    shift = 15;
    break;

  default:
    return;
  }

  if (e.checked) {
    emulator.state.registers = Object.assign({}, emulator.state.registers, { flags: emulator.state.registers.flags | 1 << shift });
  } else {
    emulator.state.registers = Object.assign({}, emulator.state.registers, { flags: emulator.state.registers.flags & ~(1 << shift) });
  }

  updateDisplay();
}

function updateMemoryBase(a: number): void {
  if (a < 0x00000) a = 0x00000;
  if (a > 0xfff00) a = 0xfff00;
  a -= a % 16;

  (<HTMLInputElement> emulator.ui.elements.namedItem("memory-base")).value = a.toString(16).toUpperCase().padStart(5, "0");

  const rows: NodeList = document.getElementById("memory-view")!.querySelectorAll("tbody tr");
  for (let i: number = 0; i < 16; i++) {
    const row: Element = <Element> rows.item(i);
    row.querySelector("th")!.innerHTML = (a + i * 0x10).toString(16).toUpperCase().padStart(5, "0");
  }

  emulator.base.memory = a;
}

function updateProgramBase(a: number): void {
  if (a < 0x00000) a = 0x00000;
  if (a > 0xfff00) a = 0xfff00;
  a -= a % 16;

  (<HTMLInputElement> emulator.ui.elements.namedItem("program-base")).value = a.toString(16).toUpperCase().padStart(5, "0");

  const rows: NodeList = document.getElementById("program-view")!.querySelectorAll("tbody tr");
  for (let i: number = 0; i < 16; i++) {
    const row: Element = <Element> rows.item(i);
    row.querySelector("th")!.innerHTML = (a + i * 0x10).toString(16).toUpperCase().padStart(5, "0");
  }

  emulator.base.program = a;
}

function updateIoReadsBase(a: number): void {
  if (a < 0x0000) a = 0x0000;
  if (a > 0xff00) a = 0xff00;
  a -= a % 16;

  (<HTMLInputElement> emulator.ui.elements.namedItem("io-reads-base")).value = a.toString(16).toUpperCase().padStart(4, "0");

  const rows: NodeList = document.getElementById("io-reads-view")!.querySelectorAll("tbody tr");
  for (let i: number = 0; i < 16; i++) {
    const row: Element = <Element> rows.item(i);
    row.querySelector("th")!.innerHTML = (a + i * 0x10).toString(16).toUpperCase().padStart(4, "0");
  }

  emulator.base.io.reads = a;
}

function updateIoWritesBase(a: number): void {
  if (a < 0x0000) a = 0x0000;
  if (a > 0xff00) a = 0xff00;
  a -= a % 16;

  (<HTMLInputElement> emulator.ui.elements.namedItem("io-writes-base")).value = a.toString(16).toUpperCase().padStart(4, "0");

  const rows: NodeList = document.getElementById("io-writes-view")!.querySelectorAll("tbody tr");
  for (let i: number = 0; i < 16; i++) {
    const row: Element = <Element> rows.item(i);
    row.querySelector("th")!.innerHTML = (a + i * 0x10).toString(16).toUpperCase().padStart(4, "0");
  }

  emulator.base.io.writes = a;
}

function updateDisplay(): void {
  const execState: HTMLOutputElement = <HTMLOutputElement> emulator.ui.elements.namedItem("exec-state");
  if (!emulator.execState.error) {
    if (emulator.execState.run) {
      (<Element> emulator.ui.elements.namedItem("run")).classList.add("hidden");
      (<Element> emulator.ui.elements.namedItem("stop")).classList.remove("hidden");
      execState.classList.replace("status-stop","status-run");
    } else {
      (<Element> emulator.ui.elements.namedItem("run")).classList.remove("hidden");
      (<Element> emulator.ui.elements.namedItem("stop")).classList.add("hidden");
      execState.classList.replace("status-run", "status-stop");
      execState.classList.replace("status-halt", "status-stop");
    }
    if (emulator.execState.halt) execState.classList.replace("status-run", "status-halt");
    execState.value = `${emulator.execState.run ? "Running" : "Stopped"}${emulator.execState.halt ? " (Halted)" : ""}`;
  } else {
    (<Element> emulator.ui.elements.namedItem("run")).classList.remove("hidden");
    (<Element> emulator.ui.elements.namedItem("stop")).classList.add("hidden");
    execState.classList.replace("status-run", "status-stop");
    execState.classList.replace("status-halt", "status-stop");
    execState.value = emulator.execState.error;
  }

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
  for (let i: number = 0; i < 16; i++) {
    (<HTMLInputElement> emulator.ui.elements.namedItem("flags" + i.toString())).checked = (emulator.state.registers.flags >> i & 1) !== 0;
  }

  {
    const rows: NodeList = document.getElementById("memory-view")!.querySelectorAll("tbody tr");
    for (let i: number = 0; i < 16; i++) {
      const cells: NodeList = (<Element> rows.item(i)).querySelectorAll("td input");
      for (let j: number = 0; j < 16; j++) {
        const cell: HTMLInputElement = <HTMLInputElement> cells.item(j);
        cell.value = emulator.memory[emulator.base.memory + i * 16 + j]!.toString(16).toUpperCase().padStart(2, "0");
      }
    }
  }

  {
    const rows: NodeList = document.getElementById("program-view")!.querySelectorAll("tbody tr");
    for (let i: number = 0; i < 16; i++) {
      const cells: NodeList = (<Element> rows.item(i)).querySelectorAll("td input");
      for (let j: number = 0; j < 16; j++) {
        const cell: HTMLInputElement = <HTMLInputElement> cells.item(j);
        cell.value = emulator.program[emulator.base.program + i * 16 + j]!.toString(16).toUpperCase().padStart(2, "0");
      }
    }
  }

  {
    const rows: NodeList = document.getElementById("io-reads-view")!.querySelectorAll("tbody tr");
    for (let i: number = 0; i < 16; i++) {
      const cells: NodeList = (<Element> rows.item(i)).querySelectorAll("td input");
      for (let j: number = 0; j < 16; j++) {
        const cell: HTMLInputElement = <HTMLInputElement> cells.item(j);
        cell.value = emulator.io.reads[emulator.base.io.reads + i * 16 + j]!.toString(16).toUpperCase().padStart(2, "0");
      }
    }
  }

  {
    const rows: NodeList = document.getElementById("io-writes-view")!.querySelectorAll("tbody tr");
    for (let i: number = 0; i < 16; i++) {
      const cells: NodeList = (<Element> rows.item(i)).querySelectorAll("td input");
      for (let j: number = 0; j < 16; j++) {
        const cell: HTMLInputElement = <HTMLInputElement> cells.item(j);
        cell.value = emulator.io.writes[emulator.base.io.writes + i * 16 + j]!.toString(16).toUpperCase().padStart(2, "0");
      }
    }
  }
}

function stepEmulator(): void {
  if (emulator.execState.halt) return;
  switch (w86.w86CpuStep(emulator.state)) {
  case w86.W86Status.SUCCESS:
    break;

  case w86.W86Status.HALT:
    emulator.execState.halt = true;
    break;

  case w86.W86Status.UNDEFINED_OPCODE:
    emulator.execState.run = false;
    emulator.execState.error = `Undefined opcode at 0x${(((emulator.state.registers.cs << 4) + emulator.state.registers.ip) % (1 << 20)).toString(16).toUpperCase().padStart(5, "0")}`;
    console.warn(emulator.execState.error);
    break;

  case w86.W86Status.UNIMPLEMENTED_OPCODE:
    emulator.execState.run = false;
    emulator.execState.error = `Unimplemented opcode at 0x${(((emulator.state.registers.cs << 4) + emulator.state.registers.ip) % (1 << 20)).toString(16).toUpperCase().padStart(5, "0")}`;
    console.error(emulator.execState.error);
    break;

  case w86.W86Status.INVALID_OPERATION:
    emulator.execState.run = false;
    emulator.execState.error = `Invalid operation at 0x${(((emulator.state.registers.cs << 4) + emulator.state.registers.ip) % (1 << 20)).toString(16).toUpperCase().padStart(5, "0")}`;
    console.warn(emulator.execState.error);
    break;

  default:
    emulator.execState.run = false;
    emulator.execState.error = "Unknown error";
    console.error(emulator.execState.error);
  }
}

function runEmulator(): void {
  if (emulator.execState.run) {
    stepEmulator();
    updateDisplay();
    setTimeout(runEmulator, 5);
  }
}

function resetEmulator(): void {
  emulator.execState = {
    run: false,
    halt: false
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
}

function restartEmulator(): void {
  resetEmulator();
  emulator.memory.set(emulator.program);
  emulator.io.reads.fill(0);
  emulator.io.writes.fill(0);
}

{
  const rows: NodeList = document.getElementById("memory-view")!.querySelectorAll("tbody tr");
  const byte: HTMLInputElement = document.createElement("input");
  byte.type = "text";
  byte.autocomplete = "off";
  byte.required = true;
  byte.size = 2;
  byte.maxLength = 2;
  byte.pattern = "[\\dA-Fa-f]*";
  byte.placeholder = "00";
  byte.value = "00";
  for (let i: number = 0; i < 16; i++) {
    for (let j: number = 0; j < 16; j++) {
      const newByte: Node = byte.cloneNode();
      newByte.addEventListener("change", (event: Event): void => {
        const e: HTMLInputElement = <HTMLInputElement> event.currentTarget;
        if (!e.checkValidity()) {
          updateDisplay();
          return;
        }

        emulator.memory[emulator.base.memory + i * 16 + j] = parseInt(e.value, 16);

        updateDisplay();
      });

      const cell: Node = document.createElement("td");
      cell.appendChild(newByte);
      rows.item(i)?.appendChild(cell);
    }
  }
}

{
  const rows: NodeList = document.getElementById("program-view")!.querySelectorAll("tbody tr");
  const byte: HTMLInputElement = document.createElement("input");
  byte.type = "text";
  byte.disabled = true;
  byte.autocomplete = "off";
  byte.required = true;
  byte.size = 2;
  byte.maxLength = 2;
  byte.pattern = "[\\dA-Fa-f]*";
  byte.placeholder = "00";
  byte.value = "00";
  for (let i: number = 0; i < 16; i++) {
    for (let j: number = 0; j < 16; j++) {
      const cell: Node = document.createElement("td");
      cell.appendChild(byte.cloneNode());
      rows.item(i)?.appendChild(cell);
    }
  }
}

{
  const rows: NodeList = document.getElementById("io-reads-view")!.querySelectorAll("tbody tr");
  const byte: HTMLInputElement = document.createElement("input");
  byte.type = "text";
  byte.autocomplete = "off";
  byte.required = true;
  byte.size = 2;
  byte.maxLength = 2;
  byte.pattern = "[\\dA-Fa-f]*";
  byte.placeholder = "00";
  byte.value = "00";
  for (let i: number = 0; i < 16; i++) {
    for (let j: number = 0; j < 16; j++) {
      const newByte: Node = byte.cloneNode();
      newByte.addEventListener("change", (event: Event): void => {
        const e: HTMLInputElement = <HTMLInputElement> event.currentTarget;
        if (!e.checkValidity()) {
          updateDisplay();
          return;
        }

        emulator.io.reads[emulator.base.io.reads + i * 16 + j] = parseInt(e.value, 16);

        updateDisplay();
      });

      const cell: Node = document.createElement("td");
      cell.appendChild(newByte);
      rows.item(i)?.appendChild(cell);
    }
  }
}

{
  const rows: NodeList = document.getElementById("io-writes-view")!.querySelectorAll("tbody tr");
  const byte: HTMLInputElement = document.createElement("input");
  byte.type = "text";
  byte.disabled = true;
  byte.autocomplete = "off";
  byte.required = true;
  byte.size = 2;
  byte.maxLength = 2;
  byte.pattern = "[\\dA-Fa-f]*";
  byte.placeholder = "00";
  byte.value = "00";
  for (let i: number = 0; i < 16; i++) {
    for (let j: number = 0; j < 16; j++) {
      const cell: Node = document.createElement("td");
      cell.appendChild(byte.cloneNode());
      rows.item(i)?.appendChild(cell);
    }
  }
}

const w86: MainModule = await W86();

const emulator: Emulator = {
  state: new w86.W86CpuState(),
  memory: new Uint8Array(),
  program: new Uint8Array(),
  io: {
    reads: new Uint8Array(),
    writes: new Uint8Array(),
  },
  execState: {
    run: false,
    halt: false
  },
  memorySize: 1048576,
  ioSize: 65536,
  base: {
    memory: 0x00000,
    program: 0x00000,
    io: {
      reads: 0x0000,
      writes: 0x0000
    }
  },
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
emulator.state.io = {
  reads: w86._malloc(emulator.ioSize),
  writes: w86._malloc(emulator.ioSize)
};
emulator.memory = w86.HEAPU8.subarray(emulator.state.memory, emulator.state.memory + emulator.memorySize).fill(0);
emulator.program = new Uint8Array(new ArrayBuffer(emulator.memorySize));
emulator.io.reads = w86.HEAPU8.subarray(emulator.state.io.reads, emulator.state.io.reads + emulator.ioSize).fill(0);
emulator.io.writes = w86.HEAPU8.subarray(emulator.state.io.writes, emulator.state.io.writes + emulator.ioSize).fill(0);

(<Element> emulator.ui.elements.namedItem("run")).addEventListener("click", (): void => {
  emulator.execState.run = true;
  setTimeout(runEmulator, 5);
});

(<Element> emulator.ui.elements.namedItem("stop")).addEventListener("click", (): void => {
  emulator.execState.run = false;
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("step")).addEventListener("click", (): void => {
  emulator.execState.run = false;
  stepEmulator();
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("reset")).addEventListener("click", (): void => {
  resetEmulator();
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("restart")).addEventListener("click", (): void => {
  restartEmulator();
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("example")).addEventListener("change", (event: Event): void => {
  const e: HTMLSelectElement = <HTMLSelectElement> event.currentTarget;
  if (!e.value) return;
  fetch(`test/${e.value}.bin`).then((res: Response): void => {
    if (!res.ok) throw new Error(`Got ${res.status} ${res.statusText} when requesting ${res.url}`);
    res.arrayBuffer().then((buf: ArrayBuffer): void => {
      emulator.program.fill(0).set(new Uint8Array(buf).subarray(0, emulator.memorySize));
      restartEmulator();
      updateDisplay();
    });
  });
});

(<Element> emulator.ui.elements.namedItem("rom")).addEventListener("change", (event: Event): void => {
  (<HTMLInputElement> event.currentTarget).files?.item(0)?.arrayBuffer().then((buf: ArrayBuffer): void => {
    (<HTMLSelectElement> emulator.ui.elements.namedItem("example")).value = "";
    emulator.program.fill(0).set(new Uint8Array(buf).subarray(0, emulator.memorySize));
    restartEmulator();
    updateDisplay();
  });
});

(<Element> emulator.ui.elements.namedItem("ax")).addEventListener("change", modifyRegister);
(<Element> emulator.ui.elements.namedItem("bx")).addEventListener("change", modifyRegister);
(<Element> emulator.ui.elements.namedItem("cx")).addEventListener("change", modifyRegister);
(<Element> emulator.ui.elements.namedItem("dx")).addEventListener("change", modifyRegister);
(<Element> emulator.ui.elements.namedItem("si")).addEventListener("change", modifyRegister);
(<Element> emulator.ui.elements.namedItem("di")).addEventListener("change", modifyRegister);
(<Element> emulator.ui.elements.namedItem("sp")).addEventListener("change", modifyRegister);
(<Element> emulator.ui.elements.namedItem("bp")).addEventListener("change", modifyRegister);
(<Element> emulator.ui.elements.namedItem("cs")).addEventListener("change", modifyRegister);
(<Element> emulator.ui.elements.namedItem("ds")).addEventListener("change", modifyRegister);
(<Element> emulator.ui.elements.namedItem("es")).addEventListener("change", modifyRegister);
(<Element> emulator.ui.elements.namedItem("ss")).addEventListener("change", modifyRegister);
(<Element> emulator.ui.elements.namedItem("ip")).addEventListener("change", modifyRegister);

(<Element> emulator.ui.elements.namedItem("flags0")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags1")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags2")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags3")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags4")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags5")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags6")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags7")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags8")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags9")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags10")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags11")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags12")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags13")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags14")).addEventListener("change", modifyFlags);
(<Element> emulator.ui.elements.namedItem("flags15")).addEventListener("change", modifyFlags);

(<Element> emulator.ui.elements.namedItem("memory-base")).addEventListener("change", (event: Event): void => {
  const e: HTMLInputElement = <HTMLInputElement> event.currentTarget;
  let a: number;
  if (!e.checkValidity()) {
    a = 0x00000;
  } else {
    a = parseInt(e.value, 16);
  }
  updateMemoryBase(a);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("memory-first")).addEventListener("click", (): void => {
  updateMemoryBase(0x00000);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("memory-prev")).addEventListener("click", (): void => {
  updateMemoryBase(emulator.base.memory - 0x100);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("memory-next")).addEventListener("click", (): void => {
  updateMemoryBase(emulator.base.memory + 0x100);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("memory-last")).addEventListener("click", (): void => {
  updateMemoryBase(0xfff00);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("program-base")).addEventListener("change", (event: Event): void => {
  const e: HTMLInputElement = <HTMLInputElement> event.currentTarget;
  let a: number;
  if (!e.checkValidity()) {
    a = 0x00000;
  } else {
    a = parseInt(e.value, 16);
  }
  updateProgramBase(a);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("program-first")).addEventListener("click", (): void => {
  updateProgramBase(0x00000);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("program-prev")).addEventListener("click", (): void => {
  updateProgramBase(emulator.base.program - 0x100);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("program-next")).addEventListener("click", (): void => {
  updateProgramBase(emulator.base.program + 0x100);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("program-last")).addEventListener("click", (): void => {
  updateProgramBase(0xfff00);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("io-reads-base")).addEventListener("change", (event: Event): void => {
  const e: HTMLInputElement = <HTMLInputElement> event.currentTarget;
  let a: number;
  if (!e.checkValidity()) {
    a = 0x0000;
  } else {
    a = parseInt(e.value, 16);
  }
  updateIoReadsBase(a);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("io-reads-first")).addEventListener("click", (): void => {
  updateIoReadsBase(0x0000);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("io-reads-prev")).addEventListener("click", (): void => {
  updateIoReadsBase(emulator.base.io.reads - 0x100);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("io-reads-next")).addEventListener("click", (): void => {
  updateIoReadsBase(emulator.base.io.reads + 0x100);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("io-reads-last")).addEventListener("click", (): void => {
  updateIoReadsBase(0xff00);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("io-writes-base")).addEventListener("change", (event: Event): void => {
  const e: HTMLInputElement = <HTMLInputElement> event.currentTarget;
  let a: number;
  if (!e.checkValidity()) {
    a = 0x0000;
  } else {
    a = parseInt(e.value, 16);
  }
  updateIoWritesBase(a);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("io-writes-first")).addEventListener("click", (): void => {
  updateIoWritesBase(0x0000);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("io-writes-prev")).addEventListener("click", (): void => {
  updateIoWritesBase(emulator.base.io.writes - 0x100);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("io-writes-next")).addEventListener("click", (): void => {
  updateIoWritesBase(emulator.base.io.writes + 0x100);
  updateDisplay();
});

(<Element> emulator.ui.elements.namedItem("io-writes-last")).addEventListener("click", (): void => {
  updateIoWritesBase(0xff00);
  updateDisplay();
});
