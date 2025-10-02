// SPDX-License-Identifier: GPL-3.0-or-later

import W86, { type MainModule } from "./w86.js"

function updateAddress(a: number): void {
  if (a < 0x00000) a = 0x00000;
  if (a > 0xfff00) a = 0xfff00;
  a -= a % 16;

  (<HTMLInputElement> emulator.elements.namedItem("base-address")).value = a.toString(16).toUpperCase().padStart(5, "0");

  const rows: NodeList = document.getElementById("memory-view")?.querySelectorAll("tbody tr")!;
  for (let i: number = 0; i < 16; i++) {
    const row: Element = <Element> rows.item(i);
    row.querySelector("th")!.innerHTML = (a + i * 0x10).toString(16).toUpperCase().padStart(5, "0");
  }

  address = a;
}

const w86: MainModule = await W86();

const emulator: HTMLFormElement = <HTMLFormElement> document.getElementById("emulator");
let address: number = 0x00000;

(<Element> emulator.elements.namedItem("base-address")).addEventListener("change", (event: Event): void => {
  const e: HTMLInputElement = <HTMLInputElement> event.currentTarget;
  let a: number;
  if (!e.checkValidity()) {
    a = 0x00000;
  } else {
    a = parseInt(e.value, 16);
  }
  updateAddress(a);
});

(<Element> emulator.elements.namedItem("first-address")).addEventListener("click", (): void => updateAddress(0x00000));

(<Element> emulator.elements.namedItem("prev-address")).addEventListener("click", (): void => updateAddress(address - 0x100));

(<Element> emulator.elements.namedItem("next-address")).addEventListener("click", (): void => updateAddress(address + 0x100));

(<Element> emulator.elements.namedItem("last-address")).addEventListener("click", (): void => updateAddress(0xfff00));

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
