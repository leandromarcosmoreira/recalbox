#!/usr/bin/env bash

set -euo pipefail

BACKUP_ROOT="${HOME}/backup"
BACKUP_DIR="${BACKUP_ROOT}/recalbox-config"

echo "== Backup de configurações do Recalbox =="

mkdir -p "${BACKUP_DIR}"

echo "Procurando cartão microSD do Recalbox montado..."

# Permite informar manualmente o ponto de montagem, se desejado:
#   ./backup_recalbox_config.sh /caminho/do/mount
USER_MOUNTPOINT="${1-}"

find_recalbox_mountpoint() {
  local mp

  # Se o usuário passou um caminho, tenta usar diretamente
  if [[ -n "${USER_MOUNTPOINT}" ]]; then
    mp="${USER_MOUNTPOINT}"
    # Se o caminho já é o diretório "share" (contém system/roms/bios diretamente)
    if [[ -d "${mp}/system" ]]; then
      echo "${mp}"
      return 0
    fi
    # Se o caminho é a partição principal que contém um diretório "share"
    if [[ -d "${mp}/share/system" ]]; then
      echo "${mp}/share"
      return 0
    fi
    echo "Erro: não foi possível localizar o diretório 'share' a partir de '${mp}'. Verifique o caminho informado." >&2
    return 1
  fi

  # Alguns caminhos padrão comuns (Recalbox em cartões SD)
  for mp in \
    "/run/media/${USER}/RECALBOX" \
    "/run/media/${USER}/SHARE" \
    "/media/${USER}/RECALBOX" \
    "/media/${USER}/SHARE"
  do
    if [[ -d "${mp}/system" ]]; then
      echo "${mp}"
      return 0
    fi
    if [[ -d "${mp}/share/system" ]]; then
      echo "${mp}/share"
      return 0
    fi
  done

  # Caso contrário, tenta descobrir automaticamente:
  # Percorre todos os MOUNTPOINTs conhecidos e procura um que contenha system/ ou share/system
  while IFS= read -r mp; do
    [[ -z "${mp}" || "${mp}" == "[SWAP]" ]] && continue
    if [[ -d "${mp}/system" ]]; then
      echo "${mp}"
      return 0
    fi
    if [[ -d "${mp}/share/system" ]]; then
      echo "${mp}/share"
      return 0
    fi
  done < <(lsblk -rno MOUNTPOINT 2>/dev/null || true)

  return 1
}

MOUNTPOINT="$(find_recalbox_mountpoint || true)"

if [[ -z "${MOUNTPOINT}" ]]; then
  # Não encontrou Recalbox montado; encerra silenciosamente para uso automático.
  exit 0
fi

echo "Encontrado Recalbox em: ${MOUNTPOINT}"

SRC_SHARE="${MOUNTPOINT}"

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

