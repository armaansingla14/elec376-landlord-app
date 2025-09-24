import React from 'react'
import API from './api'

export default function Login({onLogin}){
  const [email, setEmail] = React.useState('demo@rml.dev')
  const [password, setPassword] = React.useState('password')
  const [err, setErr] = React.useState('')
  const [busy, setBusy] = React.useState(false)

  const submit = async (e) => {
    e.preventDefault()
    setBusy(true); setErr('')
    try {
      const data = await API.login(email, password)
      onLogin(data)
    } catch (e) {
      setErr(e.message)
    } finally {
      setBusy(false)
    }
  }

  return (
    <form onSubmit={submit}>
      <div style={{display: 'grid', gap: 12}}>
        <label>Email<br/>
          <input value={email} onChange={e=>setEmail(e.target.value)} placeholder="you@example.com" />
        </label>
        <label>Password<br/>
          <input type="password" value={password} onChange={e=>setPassword(e.target.value)} placeholder="••••••••" />
        </label>
        <button disabled={busy}>{busy ? 'Signing in…' : 'Sign in'}</button>
        {err && <div style={{color:'crimson'}}>{err}</div>}
      </div>
    </form>
  )
}
