# YunHsuanHung — Personal Resume Page

An English single-page resume for **Hung Yun Hsuan (Software Engineer, Fullstack)**, built with pure **HTML + CSS + JavaScript** — no frameworks, no build step. Everything lives in a single self-contained `index.html`.

## Preview

Open `index.html` directly in any modern browser, or serve it locally:

```bash
# Python
python3 -m http.server 8080

# or Node
npx serve .
```

Then visit `http://localhost:8080`.

## Features

- **Git-log style experience timeline** — work history rendered as a commit history, with the current role tagged as `HEAD`
- **Responsive layout** — two-column (skills / experience) on desktop, single column under 820px
- **Copy email button** — one-click copy with toast confirmation (Clipboard API + fallback)
- **Print / PDF button** — dedicated `@media print` stylesheet produces a clean one-column PDF
- **Scroll-reveal animations** — via `IntersectionObserver`, automatically disabled when `prefers-reduced-motion` is set
- **Accessible** — semantic HTML, visible keyboard focus states, `aria-live` toast

## Tech

| Layer | Choice |
| ----- | ------ |
| Markup | Semantic HTML5 |
| Styling | Vanilla CSS (custom properties, grid, `@media print`) |
| Behavior | Vanilla JS (Clipboard API, IntersectionObserver) |
| Fonts | Space Grotesk · IBM Plex Sans · IBM Plex Mono (Google Fonts) |

## Structure

```
YunHsuanHung/
├── index.html   # the whole site (HTML + embedded CSS/JS)
└── README.md
```

## License

Personal resume content © Hung Yun Hsuan. Code free to reference.
