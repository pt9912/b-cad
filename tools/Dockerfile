# b-cad Doku-Validator — reproduzierbares, dependency-freies Node-Skript.
# Vendored aus dem AI-Harness-Kurs (tools/docs-check.js). Single-Purpose:
# nur der Markdown-Link-Validator (b-cad nutzt den alignment-check des
# Kurses nicht).
#
#   docker build -t bcad-docs-check tools/
#   docker run --rm -v "$PWD":/work bcad-docs-check
#
# Hinweis (Modul 14): Zielzustand ist Pinning der Base per @sha256-Digest.
# Solange b-cad keinen eigenen Image-Hash-Vertrag hat (erster Code-Slice),
# bleibt der Tag — bewusst, dokumentiert, nicht stillschweigend.

FROM node:22-alpine AS docs-check
WORKDIR /work
COPY docs-check.js /usr/local/bin/docs-check.js
RUN node --check /usr/local/bin/docs-check.js \
 && chmod +x /usr/local/bin/docs-check.js
ENTRYPOINT ["node", "/usr/local/bin/docs-check.js"]
