/*
 * Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "./upf/context.h"
#include "./upf/gtp-path.h"
#include "./upf/pfcp-path.h"
#include "./upf/metrics.h"

static ogs_thread_t *thread;
static void iupf_main(void *data);

static int initialized = 0;

int iupf_initialize(void)
{
    int rv;

    upf_metrics_init();

    ogs_gtp_context_init(OGS_MAX_NUM_OF_GTPU_RESOURCE);
    ogs_pfcp_context_init();

    upf_context_init();
    upf_event_init();
    upf_gtp_init();

    rv = ogs_pfcp_xact_init(); 
    if (rv != OGS_OK) return rv;

    rv = ogs_gtp_context_parse_config("iupf", "smf");
    if (rv != OGS_OK) return rv;

    rv = ogs_pfcp_context_parse_config("iupf", "smf");
    if (rv != OGS_OK) return rv;

    rv = ogs_metrics_context_parse_config("iupf");
    if (rv != OGS_OK) return rv;

    rv = upf_context_parse_config();
    if (rv != OGS_OK) return rv;

    rv = ogs_log_config_domain(
            ogs_app()->logger.domain, ogs_app()->logger.level);
    if (rv != OGS_OK) return rv;

    rv = ogs_pfcp_ue_pool_generate();
    if (rv != OGS_OK) return rv;

    ogs_metrics_context_open(ogs_metrics_self());

    rv = upf_pfcp_open();
    if (rv != OGS_OK) return rv;

    rv = upf_gtp_open();
    if (rv != OGS_OK) return rv;

    thread = ogs_thread_create(iupf_main, NULL);
    if (!thread) return OGS_ERROR;

    initialized = 1;

    return OGS_OK;
}

void iupf_terminate(void)
{
    if (!initialized) return;

    upf_event_term();

    ogs_thread_destroy(thread);

    upf_pfcp_close();
    upf_gtp_close();

    ogs_metrics_context_close(ogs_metrics_self());

    upf_context_final();

    ogs_pfcp_context_final();
    ogs_gtp_context_final();

    ogs_pfcp_xact_final();

    upf_gtp_final();
    upf_event_final();

    upf_metrics_final();
}

static void iupf_main(void *data)
{
    ogs_fsm_t iupf_sm;
    int rv;
    
    //I-UPF有限状态机初始化
    ogs_fsm_init(&iupf_sm, upf_state_initial, upf_state_final, 0);
    
    //polling等待消息
    for ( ;; ) {
        ogs_pollset_poll(ogs_app()->pollset,
                ogs_timer_mgr_next(ogs_app()->timer_mgr));  
                //ogs_timer_mgr_next()函数获取即将超时的定时器的剩余超时时间，并作为参数传入

        /*
         * After ogs_pollset_poll(), ogs_timer_mgr_expire() must be called.
         *
         * The reason is why ogs_timer_mgr_next() can get the corrent value
         * when ogs_timer_stop() is called internally in ogs_timer_mgr_expire().
         *
         * You should not use event-queue before ogs_timer_mgr_expire().
         * In this case, ogs_timer_mgr_expire() does not work
         * because 'if rv == OGS_DONE' statement is exiting and
         * not calling ogs_timer_mgr_expire().
         */

        //处理超时定时器
        ogs_timer_mgr_expire(ogs_app()->timer_mgr);

        for ( ;; ) {
            upf_event_t *e = NULL; //没改  iupf_event_t ? 

            // 尝试获取一个消息，如果没有消息不会阻塞，区别于pop方法
            rv = ogs_queue_trypop(ogs_app()->queue, (void**)&e);
            ogs_assert(rv != OGS_ERROR);

            if (rv == OGS_DONE)
                goto done;

            if (rv == OGS_RETRY)
                break;

            ogs_assert(e);
            //将消息送进FSM处理
            ogs_fsm_dispatch(&iupf_sm, e);
            upf_event_free(e);
        }
    }
done:

    ogs_fsm_fini(&iupf_sm, 0);
}