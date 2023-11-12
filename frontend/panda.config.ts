import { defineConfig } from "@pandacss/dev";

export default defineConfig({
  preflight: true,
  include: ["./src/**/*.{js,jsx,ts,tsx}"],
  exclude: [],
  theme: {
    extend: {
      tokens: {
        fonts: {
          inter: { value: "var(--font-inter), Helvetica, Arial, sans-serif" },
        },
      },
    },
  },
  outdir: "panda-css",
  jsxFramework: "react",
  prefix: "pd",
});
