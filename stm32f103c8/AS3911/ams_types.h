/*
 *****************************************************************************
 * Copyright by ams AG                                                       *
 * All rights are reserved.                                                  *
 *                                                                           *
 * IMPORTANT - PLEASE READ CAREFULLY BEFORE COPYING, INSTALLING OR USING     *
 * THE SOFTWARE.                                                             *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS         *
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT          *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
 *****************************************************************************
 */
/*
 *      PROJECT:   ASxxxx firmware
 *      $Revision: 1.2 $
 *      LANGUAGE:  ANSI C
 */

/*! \file
 *
 *  \author
 *
 *  \brief Basic datatypes
 *
 */

#ifndef AMS_TYPES_H
#define AMS_TYPES_H

/*!
 * Basic datatypes are mapped to ams datatypes that
 * shall be used in all ams projects.
 */
typedef unsigned char		U8;   /*!< represents an unsigned 8bit-wide type */
typedef signed char			S8;     /*!< represents a signed 8bit-wide type */
typedef unsigned short		U16;   /*!< represents an unsigned 16bit-wide type */
typedef signed short		S16;     /*!< represents a signed 16bit-wide type */
typedef unsigned long		U32;  /*!< represents an unsigned 32bit-wide type */
typedef signed long			S32;      /*!< represents a signed 32bit-wide type */
typedef unsigned long long	U64;/*!< represents an unsigned 64bit-wide type */
typedef signed long long	S64;  /*!< represents n signed 64bit-wide type */


#define U8_C(x)     (x) /*!<
Define a constant of type U8 */
#define S8_C(x)     (x) /*!<
Define a constant of type S8 */
#define U16_C(x)    (x) /*!<
Define a constant of type U16 */
#define S16_C(x)    (x) /*!<
Define a constant of type S16 */
#define U32_C(x)    (x##UL) /*!<
Define a constant of type U32 */
#define S32_C(x)    (x##L) /*!<
Define a constant of type S32 */
#define U64_C(x)    (x##ULL) /*!<
Define a constant of type U64 */
#define S64_C(x)    (x##LL) /*!<
Define a constant of type S64 */
#define UMWORD_C(x) (x) /*!<
Define a constant of type umword */
#define MWORD_C(x)  (x) /*!<
Define a constant of type mword */

typedef int bool_t; /*!<
                            represents a boolean type */

#ifndef TRUE
#define TRUE 1 /*!<
used for the #bool_t type */
#endif
#ifndef FALSE
#define FALSE 0 /*!<
used for the #bool_t type */
#endif

#ifndef NULL
#define NULL (void*)0 /*!<
    represents a NULL pointer */
#endif

#endif /* AMS_TYPES_H */

