//
// ssl/stream.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.lslboost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_SSL_STREAM_HPP
#define BOOST_ASIO_SSL_STREAM_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <lslboost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_ENABLE_OLD_SSL)
# include <lslboost/asio/ssl/old/stream.hpp>
#else // defined(BOOST_ASIO_ENABLE_OLD_SSL)
# include <lslboost/asio/async_result.hpp>
# include <lslboost/asio/detail/buffer_sequence_adapter.hpp>
# include <lslboost/asio/detail/handler_type_requirements.hpp>
# include <lslboost/asio/detail/noncopyable.hpp>
# include <lslboost/asio/detail/type_traits.hpp>
# include <lslboost/asio/ssl/context.hpp>
# include <lslboost/asio/ssl/detail/buffered_handshake_op.hpp>
# include <lslboost/asio/ssl/detail/handshake_op.hpp>
# include <lslboost/asio/ssl/detail/io.hpp>
# include <lslboost/asio/ssl/detail/read_op.hpp>
# include <lslboost/asio/ssl/detail/shutdown_op.hpp>
# include <lslboost/asio/ssl/detail/stream_core.hpp>
# include <lslboost/asio/ssl/detail/write_op.hpp>
# include <lslboost/asio/ssl/stream_base.hpp>
#endif // defined(BOOST_ASIO_ENABLE_OLD_SSL)

#include <lslboost/asio/detail/push_options.hpp>

namespace lslboost {
namespace asio {
namespace ssl {

#if defined(BOOST_ASIO_ENABLE_OLD_SSL)

using lslboost::asio::ssl::old::stream;

#else // defined(BOOST_ASIO_ENABLE_OLD_SSL)

/// Provides stream-oriented functionality using SSL.
/**
 * The stream class template provides asynchronous and blocking stream-oriented
 * functionality using SSL.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe. The application must also ensure that all
 * asynchronous operations are performed within the same implicit or explicit
 * strand.
 *
 * @par Example
 * To use the SSL stream template with an ip::tcp::socket, you would write:
 * @code
 * lslboost::asio::io_service io_service;
 * lslboost::asio::ssl::context ctx(lslboost::asio::ssl::context::sslv23);
 * lslboost::asio::ssl::stream<asio:ip::tcp::socket> sock(io_service, ctx);
 * @endcode
 *
 * @par Concepts:
 * AsyncReadStream, AsyncWriteStream, Stream, SyncReadStream, SyncWriteStream.
 */
template <typename Stream>
class stream :
  public stream_base,
  private noncopyable
{
public:
  /// The native handle type of the SSL stream.
  typedef SSL* native_handle_type;

  /// Structure for use with deprecated impl_type.
  struct impl_struct
  {
    SSL* ssl;
  };

  /// (Deprecated: Use native_handle_type.) The underlying implementation type.
  typedef impl_struct* impl_type;

  /// The type of the next layer.
  typedef typename remove_reference<Stream>::type next_layer_type;

  /// The type of the lowest layer.
  typedef typename next_layer_type::lowest_layer_type lowest_layer_type;

  /// Construct a stream.
  /**
   * This constructor creates a stream and initialises the underlying stream
   * object.
   *
   * @param arg The argument to be passed to initialise the underlying stream.
   *
   * @param ctx The SSL context to be used for the stream.
   */
  template <typename Arg>
  stream(Arg& arg, context& ctx)
    : next_layer_(arg),
      core_(ctx.native_handle(), next_layer_.lowest_layer().get_io_service())
  {
    backwards_compatible_impl_.ssl = core_.engine_.native_handle();
  }

  /// Destructor.
  ~stream()
  {
  }

  /// Get the io_service associated with the object.
  /**
   * This function may be used to obtain the io_service object that the stream
   * uses to dispatch handlers for asynchronous operations.
   *
   * @return A reference to the io_service object that stream will use to
   * dispatch handlers. Ownership is not transferred to the caller.
   */
  lslboost::asio::io_service& get_io_service()
  {
    return next_layer_.lowest_layer().get_io_service();
  }

  /// Get the underlying implementation in the native type.
  /**
   * This function may be used to obtain the underlying implementation of the
   * context. This is intended to allow access to context functionality that is
   * not otherwise provided.
   *
   * @par Example
   * The native_handle() function returns a pointer of type @c SSL* that is
   * suitable for passing to functions such as @c SSL_get_verify_result and
   * @c SSL_get_peer_certificate:
   * @code
   * lslboost::asio::ssl::stream<asio:ip::tcp::socket> sock(io_service, ctx);
   *
   * // ... establish connection and perform handshake ...
   *
   * if (X509* cert = SSL_get_peer_certificate(sock.native_handle()))
   * {
   *   if (SSL_get_verify_result(sock.native_handle()) == X509_V_OK)
   *   {
   *     // ...
   *   }
   * }
   * @endcode
   */
  native_handle_type native_handle()
  {
    return core_.engine_.native_handle();
  }

  /// (Deprecated: Use native_handle().) Get the underlying implementation in
  /// the native type.
  /**
   * This function may be used to obtain the underlying implementation of the
   * context. This is intended to allow access to stream functionality that is
   * not otherwise provided.
   */
  impl_type impl()
  {
    return &backwards_compatible_impl_;
  }

  /// Get a reference to the next layer.
  /**
   * This function returns a reference to the next layer in a stack of stream
   * layers.
   *
   * @return A reference to the next layer in the stack of stream layers.
   * Ownership is not transferred to the caller.
   */
  const next_layer_type& next_layer() const
  {
    return next_layer_;
  }

  /// Get a reference to the next layer.
  /**
   * This function returns a reference to the next layer in a stack of stream
   * layers.
   *
   * @return A reference to the next layer in the stack of stream layers.
   * Ownership is not transferred to the caller.
   */
  next_layer_type& next_layer()
  {
    return next_layer_;
  }

  /// Get a reference to the lowest layer.
  /**
   * This function returns a reference to the lowest layer in a stack of
   * stream layers.
   *
   * @return A reference to the lowest layer in the stack of stream layers.
   * Ownership is not transferred to the caller.
   */
  lowest_layer_type& lowest_layer()
  {
    return next_layer_.lowest_layer();
  }

  /// Get a reference to the lowest layer.
  /**
   * This function returns a reference to the lowest layer in a stack of
   * stream layers.
   *
   * @return A reference to the lowest layer in the stack of stream layers.
   * Ownership is not transferred to the caller.
   */
  const lowest_layer_type& lowest_layer() const
  {
    return next_layer_.lowest_layer();
  }

  /// Set the peer verification mode.
  /**
   * This function may be used to configure the peer verification mode used by
   * the stream. The new mode will override the mode inherited from the context.
   *
   * @param v A bitmask of peer verification modes. See @ref verify_mode for
   * available values.
   *
   * @throws lslboost::system::system_error Thrown on failure.
   *
   * @note Calls @c SSL_set_verify.
   */
  void set_verify_mode(verify_mode v)
  {
    lslboost::system::error_code ec;
    set_verify_mode(v, ec);
    lslboost::asio::detail::throw_error(ec, "set_verify_mode");
  }

  /// Set the peer verification mode.
  /**
   * This function may be used to configure the peer verification mode used by
   * the stream. The new mode will override the mode inherited from the context.
   *
   * @param v A bitmask of peer verification modes. See @ref verify_mode for
   * available values.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note Calls @c SSL_set_verify.
   */
  lslboost::system::error_code set_verify_mode(
      verify_mode v, lslboost::system::error_code& ec)
  {
    return core_.engine_.set_verify_mode(v, ec);
  }

  /// Set the peer verification depth.
  /**
   * This function may be used to configure the maximum verification depth
   * allowed by the stream.
   *
   * @param depth Maximum depth for the certificate chain verification that
   * shall be allowed.
   *
   * @throws lslboost::system::system_error Thrown on failure.
   *
   * @note Calls @c SSL_set_verify_depth.
   */
  void set_verify_depth(int depth)
  {
    lslboost::system::error_code ec;
    set_verify_depth(depth, ec);
    lslboost::asio::detail::throw_error(ec, "set_verify_depth");
  }

  /// Set the peer verification depth.
  /**
   * This function may be used to configure the maximum verification depth
   * allowed by the stream.
   *
   * @param depth Maximum depth for the certificate chain verification that
   * shall be allowed.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note Calls @c SSL_set_verify_depth.
   */
  lslboost::system::error_code set_verify_depth(
      int depth, lslboost::system::error_code& ec)
  {
    return core_.engine_.set_verify_depth(depth, ec);
  }

  /// Set the callback used to verify peer certificates.
  /**
   * This function is used to specify a callback function that will be called
   * by the implementation when it needs to verify a peer certificate.
   *
   * @param callback The function object to be used for verifying a certificate.
   * The function signature of the handler must be:
   * @code bool verify_callback(
   *   bool preverified, // True if the certificate passed pre-verification.
   *   verify_context& ctx // The peer certificate and other context.
   * ); @endcode
   * The return value of the callback is true if the certificate has passed
   * verification, false otherwise.
   *
   * @throws lslboost::system::system_error Thrown on failure.
   *
   * @note Calls @c SSL_set_verify.
   */
  template <typename VerifyCallback>
  void set_verify_callback(VerifyCallback callback)
  {
    lslboost::system::error_code ec;
    this->set_verify_callback(callback, ec);
    lslboost::asio::detail::throw_error(ec, "set_verify_callback");
  }

  /// Set the callback used to verify peer certificates.
  /**
   * This function is used to specify a callback function that will be called
   * by the implementation when it needs to verify a peer certificate.
   *
   * @param callback The function object to be used for verifying a certificate.
   * The function signature of the handler must be:
   * @code bool verify_callback(
   *   bool preverified, // True if the certificate passed pre-verification.
   *   verify_context& ctx // The peer certificate and other context.
   * ); @endcode
   * The return value of the callback is true if the certificate has passed
   * verification, false otherwise.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note Calls @c SSL_set_verify.
   */
  template <typename VerifyCallback>
  lslboost::system::error_code set_verify_callback(VerifyCallback callback,
      lslboost::system::error_code& ec)
  {
    return core_.engine_.set_verify_callback(
        new detail::verify_callback<VerifyCallback>(callback), ec);
  }

  /// Perform SSL handshaking.
  /**
   * This function is used to perform SSL handshaking on the stream. The
   * function call will block until handshaking is complete or an error occurs.
   *
   * @param type The type of handshaking to be performed, i.e. as a client or as
   * a server.
   *
   * @throws lslboost::system::system_error Thrown on failure.
   */
  void handshake(handshake_type type)
  {
    lslboost::system::error_code ec;
    handshake(type, ec);
    lslboost::asio::detail::throw_error(ec, "handshake");
  }

  /// Perform SSL handshaking.
  /**
   * This function is used to perform SSL handshaking on the stream. The
   * function call will block until handshaking is complete or an error occurs.
   *
   * @param type The type of handshaking to be performed, i.e. as a client or as
   * a server.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  lslboost::system::error_code handshake(handshake_type type,
      lslboost::system::error_code& ec)
  {
    detail::io(next_layer_, core_, detail::handshake_op(type), ec);
    return ec;
  }

  /// Perform SSL handshaking.
  /**
   * This function is used to perform SSL handshaking on the stream. The
   * function call will block until handshaking is complete or an error occurs.
   *
   * @param type The type of handshaking to be performed, i.e. as a client or as
   * a server.
   *
   * @param buffers The buffered data to be reused for the handshake.
   *
   * @throws lslboost::system::system_error Thrown on failure.
   */
  template <typename ConstBufferSequence>
  void handshake(handshake_type type, const ConstBufferSequence& buffers)
  {
    lslboost::system::error_code ec;
    handshake(type, buffers, ec);
    lslboost::asio::detail::throw_error(ec, "handshake");
  }

  /// Perform SSL handshaking.
  /**
   * This function is used to perform SSL handshaking on the stream. The
   * function call will block until handshaking is complete or an error occurs.
   *
   * @param type The type of handshaking to be performed, i.e. as a client or as
   * a server.
   *
   * @param buffers The buffered data to be reused for the handshake.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  template <typename ConstBufferSequence>
  lslboost::system::error_code handshake(handshake_type type,
      const ConstBufferSequence& buffers, lslboost::system::error_code& ec)
  {
    detail::io(next_layer_, core_,
        detail::buffered_handshake_op<ConstBufferSequence>(type, buffers), ec);
    return ec;
  }

  /// Start an asynchronous SSL handshake.
  /**
   * This function is used to asynchronously perform an SSL handshake on the
   * stream. This function call always returns immediately.
   *
   * @param type The type of handshaking to be performed, i.e. as a client or as
   * a server.
   *
   * @param handler The handler to be called when the handshake operation
   * completes. Copies will be made of the handler as required. The equivalent
   * function signature of the handler must be:
   * @code void handler(
   *   const lslboost::system::error_code& error // Result of operation.
   * ); @endcode
   */
  template <typename HandshakeHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(HandshakeHandler,
      void (lslboost::system::error_code))
  async_handshake(handshake_type type,
      BOOST_ASIO_MOVE_ARG(HandshakeHandler) handler)
  {
    // If you get an error on the following line it means that your handler does
    // not meet the documented type requirements for a HandshakeHandler.
    BOOST_ASIO_HANDSHAKE_HANDLER_CHECK(HandshakeHandler, handler) type_check;

    lslboost::asio::detail::async_result_init<
      HandshakeHandler, void (lslboost::system::error_code)> init(
        BOOST_ASIO_MOVE_CAST(HandshakeHandler)(handler));

    detail::async_io(next_layer_, core_,
        detail::handshake_op(type), init.handler);

    return init.result.get();
  }

  /// Start an asynchronous SSL handshake.
  /**
   * This function is used to asynchronously perform an SSL handshake on the
   * stream. This function call always returns immediately.
   *
   * @param type The type of handshaking to be performed, i.e. as a client or as
   * a server.
   *
   * @param buffers The buffered data to be reused for the handshake. Although
   * the buffers object may be copied as necessary, ownership of the underlying
   * buffers is retained by the caller, which must guarantee that they remain
   * valid until the handler is called.
   *
   * @param handler The handler to be called when the handshake operation
   * completes. Copies will be made of the handler as required. The equivalent
   * function signature of the handler must be:
   * @code void handler(
   *   const lslboost::system::error_code& error, // Result of operation.
   *   std::size_t bytes_transferred // Amount of buffers used in handshake.
   * ); @endcode
   */
  template <typename ConstBufferSequence, typename BufferedHandshakeHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(BufferedHandshakeHandler,
      void (lslboost::system::error_code, std::size_t))
  async_handshake(handshake_type type, const ConstBufferSequence& buffers,
      BOOST_ASIO_MOVE_ARG(BufferedHandshakeHandler) handler)
  {
    // If you get an error on the following line it means that your handler does
    // not meet the documented type requirements for a BufferedHandshakeHandler.
    BOOST_ASIO_BUFFERED_HANDSHAKE_HANDLER_CHECK(
        BufferedHandshakeHandler, handler) type_check;

    lslboost::asio::detail::async_result_init<BufferedHandshakeHandler,
      void (lslboost::system::error_code, std::size_t)> init(
        BOOST_ASIO_MOVE_CAST(BufferedHandshakeHandler)(handler));

    detail::async_io(next_layer_, core_,
        detail::buffered_handshake_op<ConstBufferSequence>(type, buffers),
        init.handler);

    return init.result.get();
  }

  /// Shut down SSL on the stream.
  /**
   * This function is used to shut down SSL on the stream. The function call
   * will block until SSL has been shut down or an error occurs.
   *
   * @throws lslboost::system::system_error Thrown on failure.
   */
  void shutdown()
  {
    lslboost::system::error_code ec;
    shutdown(ec);
    lslboost::asio::detail::throw_error(ec, "shutdown");
  }

  /// Shut down SSL on the stream.
  /**
   * This function is used to shut down SSL on the stream. The function call
   * will block until SSL has been shut down or an error occurs.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  lslboost::system::error_code shutdown(lslboost::system::error_code& ec)
  {
    detail::io(next_layer_, core_, detail::shutdown_op(), ec);
    return ec;
  }

  /// Asynchronously shut down SSL on the stream.
  /**
   * This function is used to asynchronously shut down SSL on the stream. This
   * function call always returns immediately.
   *
   * @param handler The handler to be called when the handshake operation
   * completes. Copies will be made of the handler as required. The equivalent
   * function signature of the handler must be:
   * @code void handler(
   *   const lslboost::system::error_code& error // Result of operation.
   * ); @endcode
   */
  template <typename ShutdownHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(ShutdownHandler,
      void (lslboost::system::error_code))
  async_shutdown(BOOST_ASIO_MOVE_ARG(ShutdownHandler) handler)
  {
    // If you get an error on the following line it means that your handler does
    // not meet the documented type requirements for a ShutdownHandler.
    BOOST_ASIO_SHUTDOWN_HANDLER_CHECK(ShutdownHandler, handler) type_check;

    lslboost::asio::detail::async_result_init<
      ShutdownHandler, void (lslboost::system::error_code)> init(
        BOOST_ASIO_MOVE_CAST(ShutdownHandler)(handler));

    detail::async_io(next_layer_, core_, detail::shutdown_op(), init.handler);

    return init.result.get();
  }

  /// Write some data to the stream.
  /**
   * This function is used to write data on the stream. The function call will
   * block until one or more bytes of data has been written successfully, or
   * until an error occurs.
   *
   * @param buffers The data to be written.
   *
   * @returns The number of bytes written.
   *
   * @throws lslboost::system::system_error Thrown on failure.
   *
   * @note The write_some operation may not transmit all of the data to the
   * peer. Consider using the @ref write function if you need to ensure that all
   * data is written before the blocking operation completes.
   */
  template <typename ConstBufferSequence>
  std::size_t write_some(const ConstBufferSequence& buffers)
  {
    lslboost::system::error_code ec;
    std::size_t n = write_some(buffers, ec);
    lslboost::asio::detail::throw_error(ec, "write_some");
    return n;
  }

  /// Write some data to the stream.
  /**
   * This function is used to write data on the stream. The function call will
   * block until one or more bytes of data has been written successfully, or
   * until an error occurs.
   *
   * @param buffers The data to be written to the stream.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @returns The number of bytes written. Returns 0 if an error occurred.
   *
   * @note The write_some operation may not transmit all of the data to the
   * peer. Consider using the @ref write function if you need to ensure that all
   * data is written before the blocking operation completes.
   */
  template <typename ConstBufferSequence>
  std::size_t write_some(const ConstBufferSequence& buffers,
      lslboost::system::error_code& ec)
  {
    return detail::io(next_layer_, core_,
        detail::write_op<ConstBufferSequence>(buffers), ec);
  }

  /// Start an asynchronous write.
  /**
   * This function is used to asynchronously write one or more bytes of data to
   * the stream. The function call always returns immediately.
   *
   * @param buffers The data to be written to the stream. Although the buffers
   * object may be copied as necessary, ownership of the underlying buffers is
   * retained by the caller, which must guarantee that they remain valid until
   * the handler is called.
   *
   * @param handler The handler to be called when the write operation completes.
   * Copies will be made of the handler as required. The equivalent function
   * signature of the handler must be:
   * @code void handler(
   *   const lslboost::system::error_code& error, // Result of operation.
   *   std::size_t bytes_transferred           // Number of bytes written.
   * ); @endcode
   *
   * @note The async_write_some operation may not transmit all of the data to
   * the peer. Consider using the @ref async_write function if you need to
   * ensure that all data is written before the blocking operation completes.
   */
  template <typename ConstBufferSequence, typename WriteHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler,
      void (lslboost::system::error_code, std::size_t))
  async_write_some(const ConstBufferSequence& buffers,
      BOOST_ASIO_MOVE_ARG(WriteHandler) handler)
  {
    // If you get an error on the following line it means that your handler does
    // not meet the documented type requirements for a WriteHandler.
    BOOST_ASIO_WRITE_HANDLER_CHECK(WriteHandler, handler) type_check;

    lslboost::asio::detail::async_result_init<
      WriteHandler, void (lslboost::system::error_code, std::size_t)> init(
        BOOST_ASIO_MOVE_CAST(WriteHandler)(handler));

    detail::async_io(next_layer_, core_,
        detail::write_op<ConstBufferSequence>(buffers), init.handler);

    return init.result.get();
  }

  /// Read some data from the stream.
  /**
   * This function is used to read data from the stream. The function call will
   * block until one or more bytes of data has been read successfully, or until
   * an error occurs.
   *
   * @param buffers The buffers into which the data will be read.
   *
   * @returns The number of bytes read.
   *
   * @throws lslboost::system::system_error Thrown on failure.
   *
   * @note The read_some operation may not read all of the requested number of
   * bytes. Consider using the @ref read function if you need to ensure that the
   * requested amount of data is read before the blocking operation completes.
   */
  template <typename MutableBufferSequence>
  std::size_t read_some(const MutableBufferSequence& buffers)
  {
    lslboost::system::error_code ec;
    std::size_t n = read_some(buffers, ec);
    lslboost::asio::detail::throw_error(ec, "read_some");
    return n;
  }

  /// Read some data from the stream.
  /**
   * This function is used to read data from the stream. The function call will
   * block until one or more bytes of data has been read successfully, or until
   * an error occurs.
   *
   * @param buffers The buffers into which the data will be read.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @returns The number of bytes read. Returns 0 if an error occurred.
   *
   * @note The read_some operation may not read all of the requested number of
   * bytes. Consider using the @ref read function if you need to ensure that the
   * requested amount of data is read before the blocking operation completes.
   */
  template <typename MutableBufferSequence>
  std::size_t read_some(const MutableBufferSequence& buffers,
      lslboost::system::error_code& ec)
  {
    return detail::io(next_layer_, core_,
        detail::read_op<MutableBufferSequence>(buffers), ec);
  }

  /// Start an asynchronous read.
  /**
   * This function is used to asynchronously read one or more bytes of data from
   * the stream. The function call always returns immediately.
   *
   * @param buffers The buffers into which the data will be read. Although the
   * buffers object may be copied as necessary, ownership of the underlying
   * buffers is retained by the caller, which must guarantee that they remain
   * valid until the handler is called.
   *
   * @param handler The handler to be called when the read operation completes.
   * Copies will be made of the handler as required. The equivalent function
   * signature of the handler must be:
   * @code void handler(
   *   const lslboost::system::error_code& error, // Result of operation.
   *   std::size_t bytes_transferred           // Number of bytes read.
   * ); @endcode
   *
   * @note The async_read_some operation may not read all of the requested
   * number of bytes. Consider using the @ref async_read function if you need to
   * ensure that the requested amount of data is read before the asynchronous
   * operation completes.
   */
  template <typename MutableBufferSequence, typename ReadHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler,
      void (lslboost::system::error_code, std::size_t))
  async_read_some(const MutableBufferSequence& buffers,
      BOOST_ASIO_MOVE_ARG(ReadHandler) handler)
  {
    // If you get an error on the following line it means that your handler does
    // not meet the documented type requirements for a ReadHandler.
    BOOST_ASIO_READ_HANDLER_CHECK(ReadHandler, handler) type_check;

    lslboost::asio::detail::async_result_init<
      ReadHandler, void (lslboost::system::error_code, std::size_t)> init(
        BOOST_ASIO_MOVE_CAST(ReadHandler)(handler));

    detail::async_io(next_layer_, core_,
        detail::read_op<MutableBufferSequence>(buffers), init.handler);

    return init.result.get();
  }

private:
  Stream next_layer_;
  detail::stream_core core_;
  impl_struct backwards_compatible_impl_;
};

#endif // defined(BOOST_ASIO_ENABLE_OLD_SSL)

} // namespace ssl
} // namespace asio
} // namespace lslboost

#include <lslboost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_SSL_STREAM_HPP
