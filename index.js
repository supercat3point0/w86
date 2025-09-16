// SPDX-License-Identifier: GPL-3.0-or-later
import W86, {} from "./w86.js";
const w86 = await W86();
document.getElementById("calculate")?.addEventListener("click", () => {
    const calculation = document.getElementById("calculation");
    const x = calculation.elements.namedItem("x");
    const y = calculation.elements.namedItem("y");
    const z = calculation.elements.namedItem("z");
    const m = new w86.Multiplication();
    m.x = x.valueAsNumber;
    m.y = y.valueAsNumber;
    w86.multiply(m);
    z.value = String(m.z);
    m.delete();
});
