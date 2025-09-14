// SPDX-License-Identifier: GPL-3.0-or-later
import W86, {} from "./w86.js";
const w86 = await W86();
document.getElementById("calculate")?.addEventListener("click", () => {
    const calculation = document.getElementById("calculation");
    const x = calculation.elements.namedItem("x");
    const y = calculation.elements.namedItem("y");
    const z = calculation.elements.namedItem("z");
    z.value = String(w86._multiply(x.valueAsNumber, y.valueAsNumber));
});
