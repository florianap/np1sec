/**
 * Multiparty Off-the-Record Messaging library
 * Copyright (C) 2014, eQualit.ie
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of version 3 of the GNU Lesser General
 * Public License as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <cstdio>
#include "src/crypt.h"


#ifndef SRC_CRYPT_CC_
#define SRC_CRYPT_CC_

gcry_error_t Hash(const void *buffer, size_t buffer_len, HashBlock hb,
                  bool secure) {
  gcry_error_t err = 0;
  gcry_md_hd_t digest = nullptr;
  unsigned int flags = 0;
  unsigned char *hash_str = nullptr;

  if (secure)
    flags |= GCRY_MD_FLAG_SECURE;

  assert(!gcry_md_test_algo(c_mpseq_hash));
  err = gcry_md_open(&digest, c_mpseq_hash, flags);
  if (err)
    goto done;

  gcry_md_write(digest, buffer, buffer_len);
  hash_str = gcry_md_read(digest, c_mpseq_hash);
  assert(hash_str);
  memcpy(hb, hash_str, sizeof(HashBlock));

done:
  gcry_md_close(digest);
  return err;
}


Ed25519Key::Ed25519Key() {
  /* Generate a new Ed25519 key pair. */
  gcry_error_t err = 0;
  gcry_sexp_t ed25519_parms;

  err = gcry_sexp_build(&ed25519_parms, NULL,
                        "(genkey (ecc (curve ed25519) (flag eddsa)))");
  if (err)
    std::printf("gcrypt: failed to create ed25519 params\n");

  err = gcry_pk_genkey(&ed25519_keypair, ed25519_parms);
  if (err)
    std::printf("gcrypt: failed to create ed25519 key pair\n");

  pub_key = gcry_sexp_find_token( key, "public-key", 0 );
  if ( !pub_key ) {
    std:printf("ed25519Key: failed to retrieve public key");
  }
  
  prv_key = gcry_sexp_find_token( key, "private-key", 0 );
  if ( !pub_key ) {
    std:printf("ed25519Key: failed to retrieve private key");
  }


}

std::string Ed25519Key::retrieveResult( gcry_sexp_t text_sexp ){
  size_t buffer_size = gcry_sexp_sprint (text_sexp, GCRYSEXP_FMT_ADVANCED,
                                            NULL, 0);
  if(!buffer_size){
    std:printf("ed25519Key: failed to convert s-expression to string");
    return NULL;
  }
  char* buffer = (char *) malloc(buffer_size);
  size_t buffer_size = gcry_sexp_sprint (test_sexp, GCRYSEXP_FMT_ADVANCED,
                                            buffer, buffer_size);

  std::string result = buffer;
  free(buffer);
  return result;
}

gcry_sexp_t Ed25519Key::ConvertToSexp(std::string text){
  gcry_error_t err = 0;
  gcry_sexp_t new_sexp;

  err = gcry_pk_new( &new_sexp, text.c_str(), 
                                      text.size(), 1);
  if( err ){
    std:printf("ed25519Key: failed to convert plain_text to gcry_sexp_t");
  }

  return new_sexp;
}

std::string Ed25519Key::Sign( std::string plain_text ){
  gcry_error_t err = 0;
  gcry_sexp_t plain_sexp, r_sig;

  plain_sexp = ConvertToSexp( plain_text );

  err = gcry_pk_sign( &r_sig, plain_sexp, prv_key ); 

  if( err ){
    std:printf("ed25519Key: failed to sign plain_text");
  }

  return retrieveResult( r_sig );
}

bool Ed25519Key::Verify( std::string signed_text, std::string sig ){
  gcry_error_t err = 0;
  gcry_sexp_t signed_sexp, sig;

  signed_sexp = ConvertToSexp(signed_text);
  sig = ConvertToSexp(sig);

  err = gcry_verify_sign( sig, signed_sexp, pub_key ); 

  if( err ){
    std:printf("ed25519Key: failed to verify signed_text");
    return false;
  }

  return true;
}

std::string Ed25519Key::Encrypt(std::string plain_text){
  gcry_sexp_t plain_sexp, crypt_sexp;
  gcry_error_t err = 0;
  
  plain_sexp = ConvertToSexp( plain_text );

  err = gcry_pk_encrypt( &crypt_sexp, plain_sexp, pub_key )
  if( err ){
    std::printf("ed25519Key: Encryption of message failed");
  }

  return retrieveResult( crypt_sexp );
}

std::string Ed25519Key::Decrypt(std::string encrypted_text){
  gcry_sexp_t crypt_sexp;
  gcry_sexp_t data_decrypted = NULL;
  gcry_error_t err = 0;

  crypt_sexp = ConvertToSexp( encrypted_text );

  err = gcry_pk_decrypt( &data_decrypted, crypt_sexp, prv_key )
  if ( err ) {
    std::printf("ed25519Key: failed to decrypt message");
  }

  return retrieveResult( data_decrypted );
}

#endif  // SRC_CRYPT_CC_