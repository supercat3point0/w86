// SPDX-License-Identifier: GPL-3.0-or-later
import W86, {} from "./w86.js";
function updateBaseAddress(a) {
    if (a < 0x00000)
        a = 0x00000;
    if (a > 0xfff00)
        a = 0xfff00;
    a -= a % 16;
    emulator.ui.elements.namedItem("base-address").value = a.toString(16).toUpperCase().padStart(5, "0");
    const rows = document.getElementById("memory-view").querySelectorAll("tbody tr");
    for (let i = 0; i < 16; i++) {
        const row = rows.item(i);
        row.querySelector("th").innerHTML = (a + i * 0x10).toString(16).toUpperCase().padStart(5, "0");
    }
    emulator.baseAddress = a;
}
function updateMemoryView() {
    const rows = document.getElementById("memory-view").querySelectorAll("tbody tr");
    for (let i = 0; i < 16; i++) {
        const cells = rows.item(i).querySelectorAll("td input");
        for (let j = 0; j < 16; j++) {
            const cell = cells.item(j);
            cell.value = emulator.memory[emulator.baseAddress + i * 16 + j].toString(16).toUpperCase().padStart(2, "0");
        }
    }
}
function updateDisplay() {
    const execState = emulator.ui.elements.namedItem("exec-state");
    if (emulator.execState.run) {
        emulator.ui.elements.namedItem("run").classList.add("hidden");
        emulator.ui.elements.namedItem("stop").classList.remove("hidden");
        execState.classList.replace("status-stop", "status-run");
    }
    else {
        emulator.ui.elements.namedItem("run").classList.remove("hidden");
        emulator.ui.elements.namedItem("stop").classList.add("hidden");
        execState.classList.replace("status-run", "status-stop");
        execState.classList.replace("status-halt", "status-stop");
    }
    if (emulator.execState.halt)
        execState.classList.replace("status-run", "status-halt");
    execState.value = `${emulator.execState.run ? "Running" : "Stopped"}${emulator.execState.halt ? " (Halted)" : ""}`;
    emulator.ui.elements.namedItem("ax").value = emulator.state.registers.ax.toString(16).toUpperCase().padStart(4, "0");
    emulator.ui.elements.namedItem("bx").value = emulator.state.registers.bx.toString(16).toUpperCase().padStart(4, "0");
    emulator.ui.elements.namedItem("cx").value = emulator.state.registers.cx.toString(16).toUpperCase().padStart(4, "0");
    emulator.ui.elements.namedItem("dx").value = emulator.state.registers.dx.toString(16).toUpperCase().padStart(4, "0");
    emulator.ui.elements.namedItem("si").value = emulator.state.registers.si.toString(16).toUpperCase().padStart(4, "0");
    emulator.ui.elements.namedItem("di").value = emulator.state.registers.di.toString(16).toUpperCase().padStart(4, "0");
    emulator.ui.elements.namedItem("sp").value = emulator.state.registers.sp.toString(16).toUpperCase().padStart(4, "0");
    emulator.ui.elements.namedItem("bp").value = emulator.state.registers.bp.toString(16).toUpperCase().padStart(4, "0");
    emulator.ui.elements.namedItem("cs").value = emulator.state.registers.cs.toString(16).toUpperCase().padStart(4, "0");
    emulator.ui.elements.namedItem("ds").value = emulator.state.registers.ds.toString(16).toUpperCase().padStart(4, "0");
    emulator.ui.elements.namedItem("es").value = emulator.state.registers.es.toString(16).toUpperCase().padStart(4, "0");
    emulator.ui.elements.namedItem("ss").value = emulator.state.registers.ss.toString(16).toUpperCase().padStart(4, "0");
    emulator.ui.elements.namedItem("ip").value = emulator.state.registers.ip.toString(16).toUpperCase().padStart(4, "0");
    for (let i = 0; i < 16; i++) {
        emulator.ui.elements.namedItem("flags" + i.toString()).checked = (emulator.state.registers.flags >> i & 1) !== 0;
    }
    updateMemoryView();
}
function stepEmulator() {
    if (emulator.execState.halt)
        return;
    switch (w86.w86CpuStep(emulator.state)) {
        case w86.W86Status.SUCCESS:
            break;
        case w86.W86Status.HALT:
            emulator.execState.halt = true;
            break;
        case w86.W86Status.UNDEFINED_OPCODE:
            console.error(`Undefined opcode at 0x${(((emulator.state.registers.cs << 4) + emulator.state.registers.ip) % (1 << 20)).toString(16).toUpperCase().padStart(5, "0")}`);
            break;
        case w86.W86Status.UNIMPLEMENTED_OPCODE:
            console.error(`Unimplemented opcode at 0x${(((emulator.state.registers.cs << 4) + emulator.state.registers.ip) % (1 << 20)).toString(16).toUpperCase().padStart(5, "0")}`);
            break;
        case w86.W86Status.INVALID_OPERATION:
            console.error(`Invalid operation at 0x${(((emulator.state.registers.cs << 4) + emulator.state.registers.ip) % (1 << 20)).toString(16).toUpperCase().padStart(5, "0")}`);
            break;
        default:
            console.error("Unknown error");
    }
}
function runEmulator() {
    if (emulator.execState.run) {
        stepEmulator();
        updateDisplay();
        setTimeout(runEmulator, 5);
    }
}
function resetEmulator() {
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
function restartEmulator() {
    resetEmulator();
    emulator.memory.set(emulator.rom);
}
{
    const rows = document.getElementById("memory-view").querySelectorAll("tbody tr");
    const byte = document.createElement("input");
    byte.type = "text";
    byte.disabled = true;
    byte.autocomplete = "off";
    byte.required = true;
    byte.size = 2;
    byte.maxLength = 2;
    byte.pattern = "[\dA-Fa-f]*";
    byte.placeholder = "00";
    byte.value = "00";
    const cell = document.createElement("td");
    cell.appendChild(byte);
    for (const row of rows) {
        for (let i = 0; i < 16; i++) {
            row.appendChild(cell.cloneNode(true));
        }
    }
}
const w86 = await W86();
const emulator = {
    state: new w86.W86CpuState(),
    memory: new Uint8Array(),
    rom: new Uint8Array(),
    execState: {
        run: false,
        halt: false
    },
    memorySize: 1048576,
    baseAddress: 0x00000,
    ui: document.getElementById("emulator")
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
emulator.memory = w86.HEAPU8.subarray(emulator.state.memory, emulator.state.memory + emulator.memorySize).fill(0);
emulator.rom = new Uint8Array(new ArrayBuffer(emulator.memorySize));
emulator.ui.elements.namedItem("run").addEventListener("click", () => {
    emulator.execState.run = true;
    setTimeout(runEmulator, 5);
});
emulator.ui.elements.namedItem("stop").addEventListener("click", () => {
    emulator.execState.run = false;
    updateDisplay();
});
emulator.ui.elements.namedItem("step").addEventListener("click", () => {
    emulator.execState.run = false;
    stepEmulator();
    updateDisplay();
});
emulator.ui.elements.namedItem("reset").addEventListener("click", () => {
    resetEmulator();
    updateDisplay();
});
emulator.ui.elements.namedItem("restart").addEventListener("click", () => {
    restartEmulator();
    updateDisplay();
});
emulator.ui.elements.namedItem("rom").addEventListener("change", (event) => {
    event.currentTarget.files?.item(0)?.arrayBuffer().then((buf) => {
        emulator.rom.fill(0).set(new Uint8Array(buf).subarray(0, emulator.memorySize));
        restartEmulator();
        updateDisplay();
    });
});
emulator.ui.elements.namedItem("base-address").addEventListener("change", (event) => {
    const e = event.currentTarget;
    let a;
    if (!e.checkValidity()) {
        a = 0x00000;
    }
    else {
        a = parseInt(e.value, 16);
    }
    updateBaseAddress(a);
    updateMemoryView();
});
emulator.ui.elements.namedItem("first-address").addEventListener("click", () => {
    updateBaseAddress(0x00000);
    updateMemoryView();
});
emulator.ui.elements.namedItem("prev-address").addEventListener("click", () => {
    updateBaseAddress(emulator.baseAddress - 0x100);
    updateMemoryView();
});
emulator.ui.elements.namedItem("next-address").addEventListener("click", () => {
    updateBaseAddress(emulator.baseAddress + 0x100);
    updateMemoryView();
});
emulator.ui.elements.namedItem("last-address").addEventListener("click", () => {
    updateBaseAddress(0xfff00);
    updateMemoryView();
});
//# sourceMappingURL=index.js.map