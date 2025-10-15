// SPDX-License-Identifier: GPL-3.0-or-later
import W86, {} from "./w86.js";
function updateAddress(a) {
    if (a < 0x00000)
        a = 0x00000;
    if (a > 0xfff00)
        a = 0xfff00;
    a -= a % 16;
    emulator.form.elements.namedItem("base-address").value = a.toString(16).toUpperCase().padStart(5, "0");
    const rows = document.getElementById("memory-view")?.querySelectorAll("tbody tr");
    for (let i = 0; i < 16; i++) {
        const row = rows.item(i);
        row.querySelector("th").innerHTML = (a + i * 0x10).toString(16).toUpperCase().padStart(5, "0");
    }
    emulator.baseAddress = a;
}
const w86 = await W86();
const emulator = {
    state: new w86.W86CPUState(),
    form: document.getElementById("emulator"),
    baseAddress: 0x00000
};
emulator.state.reg = {
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
emulator.state.mem = w86._malloc(1048576);
{
    const rows = document.getElementById("memory-view")?.querySelectorAll("tbody tr");
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
emulator.form.elements.namedItem("step").addEventListener("click", () => w86.w86CPUStep(emulator.state));
emulator.form.elements.namedItem("base-address").addEventListener("change", (event) => {
    const e = event.currentTarget;
    let a;
    if (!e.checkValidity()) {
        a = 0x00000;
    }
    else {
        a = parseInt(e.value, 16);
    }
    updateAddress(a);
});
emulator.form.elements.namedItem("first-address").addEventListener("click", () => updateAddress(0x00000));
emulator.form.elements.namedItem("prev-address").addEventListener("click", () => updateAddress(emulator.baseAddress - 0x100));
emulator.form.elements.namedItem("next-address").addEventListener("click", () => updateAddress(emulator.baseAddress + 0x100));
emulator.form.elements.namedItem("last-address").addEventListener("click", () => updateAddress(0xfff00));
