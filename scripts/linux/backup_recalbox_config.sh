#!/usr/bin/env bash

set -euo pipefail

BACKUP_ROOT="${HOME}/backup"
BACKUP_DIR="${BACKUP_ROOT}/recalbox-config"

echo "== Backup de configurações do Recalbox =="

mkdir -p "${BACKUP_DIR}"

echo "Procurando cartão microSD do Recalbox montado..."

# Encontra ponto de montagem com label RECALBOX
MOUNTPOINT=""
while IFS= read -r line; do
  # lsblk -P retorna pares do tipo CHAVE="valor"
  eval "${line}"
  if [[ "${LABEL:-}" == "RECALBOX" && -n "${MOUNTPOINT:-}" ]]; then
    MOUNTPOINT="${MOUNTPOINT}"
    break
  fi
done < <(lsblk -P -o LABEL,MOUNTPOINT 2>/dev/null || true)

if [[ -z "${MOUNTPOINT}" ]]; then
  echo "Erro: não foi encontrado nenhum dispositivo montado com label 'RECALBOX'." >&2
  echo "Certifique-se de que o microSD está plugado e montado, ou monte-o manualmente e tente novamente." >&2
  exit 1
fi

echo "Encontrado Recalbox em: ${MOUNTPOINT}"

SRC_SHARE="${MOUNTPOINT}/share"

if [[ ! -d "${SRC_SHARE}" ]]; then
  echo "Erro: diretório '${SRC_SHARE}' não encontrado no cartão. A estrutura do Recalbox parece diferente." >&2
  exit 1
fi

timestamp="$(date +%Y%m%d-%H%M%S)"
DEST_DIR="${BACKUP_DIR}/${timestamp}"

mkdir -p "${DEST_DIR}"

echo "Criando backup em: ${DEST_DIR}"

copy_if_exists() {
  local src="$1"
  local dest="$2"

  if [[ -d "${src}" ]]; then
    echo "Copiando '${src}'..."
    cp -a "${src}" "${dest}/"
  else
    echo "Aviso: diretório '${src}' não encontrado, pulando."
  fi
}

# Diretórios principais de configuração e conteúdo
copy_if_exists "${SRC_SHARE}/system" "${DEST_DIR}"
copy_if_exists "${SRC_SHARE}/roms" "${DEST_DIR}"
copy_if_exists "${SRC_SHARE}/bios" "${DEST_DIR}"

echo
echo "Backup concluído."
echo "Arquivos salvos em: ${DEST_DIR}"

