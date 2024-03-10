#pragma once

#include <KTextEditor/Document>

class document_read_only_t
  {
  KTextEditor::Document * m_document;
  bool was_read_write;

public:
  explicit document_read_only_t(KTextEditor::Document & document) :
      m_document(&document),
      was_read_write(document.isReadWrite())
    {
    if(m_document)
      m_document->setReadWrite(false);
    }

  ~document_read_only_t() { clear_state(); }

  void clear_state()
    {
    if(m_document)
      {
      m_document->setReadWrite(was_read_write);
      m_document = nullptr;
      }
    }

  document_read_only_t(document_read_only_t const &) = delete;
  document_read_only_t & operator=(document_read_only_t const &) = delete;

  document_read_only_t(document_read_only_t &&) noexcept = default;
  document_read_only_t & operator=(document_read_only_t &&) noexcept = default;
  };
