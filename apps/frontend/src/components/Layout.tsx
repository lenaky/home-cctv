import { Outlet, NavLink } from 'react-router-dom'

export default function Layout() {
  return (
    <div className="min-h-screen bg-gray-950 text-gray-100">
      <header className="bg-gray-900 border-b border-gray-800 px-6 py-3 flex items-center gap-6">
        <span className="font-bold text-lg tracking-tight text-white">Home CCTV</span>
        <nav className="flex gap-4 text-sm">
          <NavLink
            to="/admin"
            className={({ isActive }) =>
              isActive ? 'text-blue-400 font-medium' : 'text-gray-400 hover:text-white'
            }
          >
            카메라 관리
          </NavLink>
        </nav>
      </header>
      <main className="p-6">
        <Outlet />
      </main>
    </div>
  )
}
