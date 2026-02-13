#!/usr/bin/env bash

set -euo pipefail

if ! command -v gh >/dev/null 2>&1; then
  echo "Erro: gh (GitHub CLI) não encontrado. Instale-o antes de continuar." >&2
  exit 1
fi

if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
  echo "Uso: $0 <dispositivo> [tag_release]" >&2
  echo "Exemplo: $0 /dev/sdX rpi5_64-main-123" >&2
  exit 1
fi

DEVICE="$1"
TAG="${2:-}"

if [ ! -b "${DEVICE}" ]; then
  echo "Erro: dispositivo de bloco '${DEVICE}' não encontrado." >&2
  exit 1
fi

echo "Resumo do dispositivo alvo:"
lsblk "${DEVICE}" || true
echo
read -r -p "ATENÇÃO: TODOS OS DADOS EM ${DEVICE} SERÃO APAGADOS. Continuar? (yes/no) " answer
if [ "${answer}" != "yes" ]; then
  echo "Abortado pelo usuário."
  exit 1
fi

REPO="$(gh repo view --json nameWithOwner --jq .nameWithOwner)"

if [ -z "${TAG}" ]; then
  echo "Obtendo última release para RPi5 (prefixo 'rpi5_64-') em ${REPO}..."
  TAG="$(gh release list --repo "${REPO}" --limit 50 --json tagName --jq '.[] | select(.tagName | startswith("rpi5_64-")) | .tagName' | head -n1 || true)"
  if [ -z "${TAG}" ]; then
    echo "Erro: nenhuma release encontrada com prefixo 'rpi5_64-'." >&2
    exit 1
  fi
fi

echo "Usando release: ${TAG}"

WORKDIR="${HOME}/Downloads/recalbox-rpi5_64"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

echo "Baixando artefatos da release..."
gh release download "${TAG}" --repo "${REPO}" --clobber

image_file="$(ls -1 *.img *.img.xz 2>/dev/null | head -n1 || true)"
if [ -z "${image_file}" ]; then
  echo "Erro: nenhuma imagem '*.img' ou '*.img.xz' encontrada após o download." >&2
  exit 1
fi

echo "Imagem selecionada: ${image_file}"
echo "Gravando em ${DEVICE} (pode demorar)..."

if [[ "${image_file}" == *.xz ]]; then
  xzcat "${image_file}" | sudo dd of="${DEVICE}" bs=4M conv=fsync status=progress
else
  sudo dd if="${image_file}" of="${DEVICE}" bs=4M conv=fsync status=progress
fi

sync
echo "Gravação concluída."

echo "Página da release:"
echo "  https://github.com/${REPO}/releases/tag/${TAG}"

