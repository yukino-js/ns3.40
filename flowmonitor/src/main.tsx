import "./global.css";

import { createRoot } from "@yukino.js/lit-jsx";
import "./app";

createRoot(document.getElementById("app")!).render(<fm-app />);
