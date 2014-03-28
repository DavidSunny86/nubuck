#include <nubuck_private.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <Nubuck\operators\operator_invoker.h>
#include <UI\window_events.h>
#include <UI\userinterface.h>
#include <world\world_events.h>
#include <world\world.h>
#include "operator_events.h"
#include "operator_driver.h"
#include "operators.h"

namespace OP {

Operators g_operators;

void Operators::UnloadModules() {
	for(unsigned i = 0; i < _ops.size(); ++i) {
        OperatorDesc& desc = _ops[i];
		if(desc.module) {
            FreeLibrary(desc.module);
			desc.module = NULL;
		}
	}
}

void Operators::Event_ActionFinished(const EV::Event& event) {
    _actionsPending--;
}

void Operators::Event_SetPanel(const EV::Event& event) {
    const EV::Params_OP_SetPanel& args = EV::def_OP_SetPanel.GetArgs(event);
    Operator* op = args.op;
    OperatorPanel* panel = NULL;
    for(unsigned i = 0; !panel && i < _ops.size(); ++i) {
        if(_ops[i].op == op) panel = _ops[i].panel;
    }
    assert(panel);
    g_ui.GetOperatorPanel().Clear();
    nubuck.ui->SetOperatorPanel(panel);
}

void Operators::Event_Default(const EV::Event& event, const char*) {
}

void Operators::OnInvokeOperator(unsigned id) {
    if(0 < _actionsPending) {
		printf("op still busy...\n");
        return;
	}

    printf("invoking operator with id = %d\n", id);
    g_ui.GetOperatorPanel().Clear();
    Operator* op = _ops[id].op;
    nubuck.ui->SetOperatorPanel(_ops[id].panel);

	if(!_driver.IsValid()) {
        _driver = GEN::MakePtr(new Driver(_activeOps, _activeOpsMtx, _meshJobs, _meshJobsMtx));
        _driver->Thread_StartAsync();
	}
	EV::Params_OP_Push args = { op };
    _ops[id].panel->Invoke();
	_driver->Send(EV::def_OP_Push.Create(args));
}

Operators::Operators() : _actionsPending(0) {
    AddEventHandler(EV::def_OP_ActionFinished, this, &Operators::Event_ActionFinished);
    AddEventHandler(EV::def_OP_SetPanel, this, &Operators::Event_SetPanel);
}

Operators::~Operators() {
    UnloadModules();
}

void Operators::FrameUpdate() {
    HandleEvents();
}

unsigned Operators::Register(OperatorPanel* panel, Operator* op, HMODULE module) {
    unsigned id = _ops.size();

    Invoker* invoker = new Invoker(id);
    connect(invoker, SIGNAL(SigInvokeOperator(unsigned)), this, SLOT(OnInvokeOperator(unsigned)));

    op->Register(nubuck, *invoker);

    OperatorDesc desc;
    desc.id = id;
    desc.op = op;
    desc.invoker = invoker;
	desc.module = module;
    desc.panel = panel;

    _ops.push_back(desc);

    return id;
}

void Operators::InvokeAction(const EV::Event& event) {
    if(0 < _actionsPending) {
		printf("op still busy...\n");
        return;
	}

    _actionsPending++;
    _driver->Send(event);
}

void Operators::InvokeEvent(const EV::Event& event) {
	_driver->Send(event);
}

void Operators::SetInitOp(unsigned id) {
    assert(_activeOps.empty());
    OnInvokeOperator(id);
}

void Operators::GetMeshJobs(std::vector<R::MeshJob>& meshJobs) {
	if(!_renderThread.IsValid()) {
        _renderThread = GEN::MakePtr(new RenderThread(_activeOps, _activeOpsMtx, _meshJobs, _meshJobsMtx));
        // _renderThread->Thread_StartAsync();
	}

    // gather jobs synchronously
    _renderThread->GatherJobs();
    
	SYS::ScopedLock lockJobs(_meshJobsMtx);
	meshJobs.insert(meshJobs.end(), _meshJobs.begin(), _meshJobs.end());
}

void Operators::OnCameraChanged() {
    if(!_actionsPending) {
        _driver->Send(EV::def_CameraChanged.Create(EV::Params_CameraChanged()));
    }
}

bool Operators::MouseEvent(const EV::Event& event) {
    if(0 < _actionsPending) W::world.Send(event);
	else _driver->Send(event);
    return true;
}

NUBUCK_API void SendToOperator(const EV::Event& event) {
    g_operators.InvokeAction(event);
}

static bool IsOperatorFilename(const std::basic_string<TCHAR>& filename) {
    bool prefix = 0 == filename.find(TEXT("alg_")) || 0 == filename.find(TEXT("op_")); 
    bool suffix = true;
    return prefix && suffix;
}

#ifdef UNICODE
#define std_tcout std::wcout
#else
#define std_tcout std::cout
#endif

void LoadOperators(void) {
    const std::string baseDir = common.BaseDir();
    std::basic_string<TCHAR> filename = std::basic_string<TCHAR>(baseDir.begin(), baseDir.end())  + TEXT("Operators\\*");
    WIN32_FIND_DATA ffd;
    HANDLE ff = FindFirstFile(filename.c_str(), &ffd);
    if(INVALID_HANDLE_VALUE == ff) {
        if(ERROR_FILE_NOT_FOUND == GetLastError()) {
            common.printf("WARNING - directory %s does not exist.\n", filename.c_str());
        } else {
            common.printf("ERROR - LoadOperators: FindFirstFile failed.\n");
            Crash();
        }
    }
    do {
        if(IsOperatorFilename(ffd.cFileName)) {
            std_tcout << TEXT("found operator ") << ffd.cFileName << std::endl;

            HMODULE lib = LoadLibrary(ffd.cFileName);
            if(!lib) {
                std_tcout << TEXT("ERROR: unable to load ") << ffd.cFileName << std::endl;
			}
			typedef OP::Operator* (*createOperator_t)();
            typedef OP::OperatorPanel* (*createPanel_t)();
            createOperator_t opFunc = (createOperator_t)GetProcAddress(lib, "CreateOperator");
            createPanel_t panelFunc = (createPanel_t)GetProcAddress(lib, "CreateOperatorPanel");
            if(!opFunc || !panelFunc) printf("ERROR - unable to load createoperator() function\n");
			else {
				OP::Operator* op = opFunc();
                OP::OperatorPanel* panel = panelFunc();
				OP::g_operators.Register(panel, op, lib);
			}
        }
    } while(FindNextFile(ff, &ffd));
}

} // namespace OP