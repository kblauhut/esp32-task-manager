import "./globals.css";
import { Inter } from "next/font/google";

// UI
import { styled } from "panda-css/jsx";

const InterFont = Inter({
  weight: ["300", "400", "500", "600"],
  display: "swap",
  subsets: ["latin"],
  variable: "--font-inter",
});

export default async function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html className={`${InterFont.variable}`} style={{ height: "100%" }}>
      <meta
        name="viewport"
        content="width=device-width, initial-scale=1.0, viewport-fit=cover"
      />
      <styled.body
        fontFamily="inter"
        bg="gray.200"
        color="#161F26"
        height="full"
      >
        {children}
      </styled.body>
    </html>
  );
}
