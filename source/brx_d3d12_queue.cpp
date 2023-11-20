//
// Copyright (C) YuqiaoZhang(HanetakaChou)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "brx_d3d12_device.h"
#include <assert.h>

brx_d3d12_graphics_queue::brx_d3d12_graphics_queue() : m_graphics_queue(NULL)
{
}

void brx_d3d12_graphics_queue::init(ID3D12CommandQueue *graphics_queue, bool uma, bool support_ray_tracing)
{
	assert(NULL == this->m_graphics_queue);
	this->m_graphics_queue = graphics_queue;

	this->m_uma = uma;

	this->m_support_ray_tracing = support_ray_tracing;
}

void brx_d3d12_graphics_queue::uninit(ID3D12CommandQueue *graphics_queue)
{
	assert(NULL != this->m_graphics_queue);

	assert(graphics_queue == this->m_graphics_queue);

	this->m_graphics_queue = NULL;
}

brx_d3d12_graphics_queue::~brx_d3d12_graphics_queue()
{
	assert(NULL == this->m_graphics_queue);
}

void brx_d3d12_graphics_queue::wait_and_submit(brx_upload_command_buffer const *brx_upload_command_buffer, brx_graphics_command_buffer const *brx_graphics_command_buffer, brx_fence *brx_fence) const
{
	assert(NULL != brx_upload_command_buffer);
	assert(NULL != brx_graphics_command_buffer);
	assert(NULL != brx_fence);
	ID3D12CommandList *upload_command_list = static_cast<brx_d3d12_upload_command_buffer const *>(brx_upload_command_buffer)->get_command_list();
	ID3D12Fence *upload_queue_submit_fence = static_cast<brx_d3d12_upload_command_buffer const *>(brx_upload_command_buffer)->get_upload_queue_submit_fence();
	ID3D12CommandList *graphics_command_list = static_cast<brx_d3d12_graphics_command_buffer const *>(brx_graphics_command_buffer)->get_command_list();
	ID3D12Fence *fence = static_cast<brx_d3d12_fence const *>(brx_fence)->get_fence();

	if ((!this->m_uma) || this->m_support_ray_tracing)
	{
		assert(NULL != upload_command_list);
		assert(NULL != upload_queue_submit_fence);

		HRESULT hr_wait = this->m_graphics_queue->Wait(upload_queue_submit_fence, 1U);
		assert(SUCCEEDED(hr_wait));

		HRESULT hr_reset = this->m_graphics_queue->Signal(upload_queue_submit_fence, 0U);
		assert(SUCCEEDED(hr_reset));

		this->m_graphics_queue->ExecuteCommandLists(1U, &graphics_command_list);

		HRESULT hr_signal = this->m_graphics_queue->Signal(fence, 1U);
		assert(SUCCEEDED(hr_signal));
	}
	else
	{
		assert(NULL == upload_command_list);
		assert(NULL == upload_queue_submit_fence);

		this->m_graphics_queue->ExecuteCommandLists(1U, &graphics_command_list);

		HRESULT hr_signal = this->m_graphics_queue->Signal(fence, 1U);
		assert(SUCCEEDED(hr_signal));
	}
}

bool brx_d3d12_graphics_queue::submit_and_present(brx_graphics_command_buffer *brx_graphics_command_buffer, brx_swap_chain *brx_swap_chain, uint32_t swap_chain_image_index, brx_fence *brx_fence) const
{
	assert(NULL != brx_graphics_command_buffer);
	assert(NULL != brx_swap_chain);
	assert(NULL != brx_fence);
	ID3D12CommandList *command_list = static_cast<brx_d3d12_graphics_command_buffer const *>(brx_graphics_command_buffer)->get_command_list();
	IDXGISwapChain3 *swap_chain = static_cast<brx_d3d12_swap_chain const *>(brx_swap_chain)->get_swap_chain();
	ID3D12Fence *fence = static_cast<brx_d3d12_fence const *>(brx_fence)->get_fence();

	this->m_graphics_queue->ExecuteCommandLists(1U, &command_list);

#if 0
	// The command list can be reset even if the present has not completed
	HRESULT hr_signal = this->m_graphics_queue->Signal(fence, 1U);
	assert(SUCCEEDED(hr_signal));

	HRESULT hr_present = swap_chain->Present(0U, 0U);
	assert(SUCCEEDED(hr_present));
#else
	HRESULT hr_present = swap_chain->Present(0U, 0U);
	assert(SUCCEEDED(hr_present));

	HRESULT hr_signal = this->m_graphics_queue->Signal(fence, 1U);
	assert(SUCCEEDED(hr_signal));
#endif

	return true;
}

brx_d3d12_upload_queue::brx_d3d12_upload_queue() : m_upload_queue(NULL)
{
}

void brx_d3d12_upload_queue::init(ID3D12CommandQueue *upload_queue, bool uma, bool support_ray_tracing)
{
	assert(NULL == this->m_upload_queue);
	this->m_upload_queue = upload_queue;

	this->m_uma = uma;

	this->m_support_ray_tracing = support_ray_tracing;
}

void brx_d3d12_upload_queue::uninit(ID3D12CommandQueue *upload_queue)
{
	assert(NULL != this->m_upload_queue);

	assert(upload_queue == this->m_upload_queue);

	this->m_upload_queue = NULL;
}

brx_d3d12_upload_queue::~brx_d3d12_upload_queue()
{
	assert(NULL == this->m_upload_queue);
}

void brx_d3d12_upload_queue::submit_and_signal(brx_upload_command_buffer const *brx_upload_command_buffer) const
{
	assert(NULL != brx_upload_command_buffer);
	ID3D12CommandList *command_list = static_cast<brx_d3d12_upload_command_buffer const *>(brx_upload_command_buffer)->get_command_list();
	ID3D12Fence *upload_queue_submit_fence = static_cast<brx_d3d12_upload_command_buffer const *>(brx_upload_command_buffer)->get_upload_queue_submit_fence();

	if ((!this->m_uma) || this->m_support_ray_tracing)
	{
		assert(NULL != command_list);
		assert(NULL != upload_queue_submit_fence);

		this->m_upload_queue->ExecuteCommandLists(1U, &command_list);

		HRESULT hr_signal = this->m_upload_queue->Signal(upload_queue_submit_fence, 1U);
		assert(SUCCEEDED(hr_signal));
	}
	else
	{
		assert(NULL == command_list);
		assert(NULL == upload_queue_submit_fence);
		assert(NULL == this->m_upload_queue);
	}
}
