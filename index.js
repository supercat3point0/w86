// SPDX-License-Identifier: GPL-3.0-or-later
import W86, {} from "./w86.js";
function updateAddress(a) {
    if (a < 0x00000)
        a = 0x00000;
    if (a > 0xfff00)
        a = 0xfff00;
    a -= a % 16;
    emulator.elements.namedItem("base-address").value = a.toString(16).toUpperCase().padStart(5, "0");
    const rows = document.getElementById("memory-view")?.querySelectorAll("tbody tr");
    for (let i = 0; i < 16; i++) {
        const row = rows.item(i);
        row.querySelector("th").innerHTML = (a + i * 0x10).toString(16).toUpperCase().padStart(5, "0");
    }
    address = a;
}
const w86 = await W86();
const emulator = document.getElementById("emulator");
let address = 0x00000;
emulator.elements.namedItem("base-address").addEventListener("change", (event) => {
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
emulator.elements.namedItem("first-address").addEventListener("click", () => updateAddress(0x00000));
emulator.elements.namedItem("prev-address").addEventListener("click", () => updateAddress(address - 0x100));
emulator.elements.namedItem("next-address").addEventListener("click", () => updateAddress(address + 0x100));
emulator.elements.namedItem("last-address").addEventListener("click", () => updateAddress(0xfff00));
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
/*document.getElementById("calculate")?.addEventListener("click", (): void => {
  const calculation: HTMLFormElement = <HTMLFormElement> document.getElementById("calculation");
  const x: HTMLInputElement = <HTMLInputElement> calculation.elements.namedItem("x");
  const y: HTMLInputElement = <HTMLInputElement> calculation.elements.namedItem("y");
  const z: HTMLOutputElement = <HTMLOutputElement> calculation.elements.namedItem("z");

  const m: Multiplication = new w86.Multiplication();
  m.x = x.valueAsNumber;
  m.y = y.valueAsNumber;
  w86.multiply(m);

  z.value = String(m.z);

  m.delete();
});*/
