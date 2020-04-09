/*
 * kvm asynchronous fault support
 *
 * Copyright 2010 Red Hat, Inc.
 *
 * Author:
 *      Gleb Natapov <gleb@redhat.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef _BISCUITOS_ASYNC_PF_H
#define _BISCUITOS_ASYNC_PF_H

#define kvm_async_pf_init_bs()		(0)
#define kvm_async_pf_deinit_bs()	do {} while (0)
#define kvm_async_pf_vcpu_init_bs()	do {} while (0)

#endif
