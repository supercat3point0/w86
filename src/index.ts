// SPDX-License-Identifier: GPL-3.0-or-later

import W86, { type WasmModule } from "./w86.js"

const w86: WasmModule = await W86();

document.getElementById("calculate")?.addEventListener("click", (): void => {
  const calculation: HTMLFormElement = <HTMLFormElement> document.getElementById("calculation");
  const x: HTMLInputElement = <HTMLInputElement> calculation.elements.namedItem("x");
  const y: HTMLInputElement = <HTMLInputElement> calculation.elements.namedItem("y");
  const z: HTMLOutputElement = <HTMLOutputElement> calculation.elements.namedItem("z");
  z.value = String(w86._multiply(x.valueAsNumber, y.valueAsNumber));
});
