from bcc import BPF


# define BPF program
bpf_text = """

int syscall__mount(struct pt_regs *ctx, char __user *dev_name, char __user *dir_name,
				char __user *type, unsigned long flags,
				void __user *data)
{
    bpf_trace_printk("hello\\n");
    return 0;
}
"""

b = BPF(text=bpf_text)
def try_attach_syscall_probes(syscall):
    syscall_fnname = b.get_syscall_fnname(syscall)
    b.attach_kprobe(event=syscall_fnname, fn_name="syscall__mount")

try_attach_syscall_probes("mount")
b.trace_print()

