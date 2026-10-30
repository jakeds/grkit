fn main() -> anyhow::Result<()> {
    let a: Vec<_> = std::env::args().collect();
    anyhow::ensure!(
        a.len() == 3,
        "Usage: soft-extract-rtos official-firmware private-output"
    );
    let decoded = grkit_build::firmware::decode(&std::fs::read(&a[1])?)?;
    std::fs::write(&a[2], grkit_build::firmware::rtos_section(&decoded)?)?;
    Ok(())
}
