// SPDX-License-Identifier: GPL-3.0-or-later

import W86, { type MainModule, type Multiplication } from "./w86.js"

const w86: MainModule = await W86();

document.getElementById("calculate")?.addEventListener("click", (): void => {
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
});
